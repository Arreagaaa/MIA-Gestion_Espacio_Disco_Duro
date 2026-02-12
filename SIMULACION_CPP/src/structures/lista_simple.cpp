/*
 * LISTA_SIMPLE.CPP
 *
 * IMPLEMENTA: Lista simplemente enlazada para gestión de disco
 *
 * CONCEPTO CLAVE:
 * En lugar de guardar TODOS los bloques (como el bitmap),
 * solo guardamos los BLOQUES LIBRES en forma de nodos.
 *
 * EJEMPLO VISUAL:
 * Disco: [✓][✓][_][_][_][✓][✓][_][_][✓]
 *                ↑_____↑        ↑__↑
 *               bloques 2-4   bloques 7-8
 *
 * Lista: [inicio:2, tam:3] → [inicio:7, tam:2] → NULL
 *
 * VENTAJA: Si el disco está 90% ocupado, solo guardamos el 10% libre.
 */

#include "./core/disk_manager.h"
#include <iostream>
#include <algorithm>

// ============================================================================
// CONSTRUCTOR
//
// PROPÓSITO: *Construir la lista inicial de bloques libres basándose en el disco.
//
// PROCESO:
// 1. Escanear el disco
// 2. Encontrar segmentos de bloques libres consecutivos
// 3. Crear un nodo por cada segmento
// ============================================================================

ListaSimple::ListaSimple() : GestorDisco(),
                             cabeza(nullptr)
{
    int inicio = -1; // Marca el inicio de un segmento libre
    int tamanio = 0; // Tamaño del segmento actual

    // Recorrer todo el disco
    for (int i = 0; i < TOTAL_BLOQUES; i++)
    {
        if (!disco[i])
        { // Bloque LIBRE
            if (inicio == -1)
            {
                // Inicio de un nuevo segmento libre
                inicio = i;
                tamanio = 1;
            }
            else
            {
                // Continuar el segmento actual
                tamanio++;
            }
        }
        else
        { // Bloque OCUPADO
            if (inicio != -1)
            {
                // Fin del segmento libre → crear nodo
                insertar_ordenado(inicio, tamanio);
                inicio = -1;
                tamanio = 0;
            }
        }
    }

    // Insertar último segmento si existe
    if (inicio != -1)
    {
        insertar_ordenado(inicio, tamanio);
    }
}

// ============================================================================
// DESTRUCTOR
//
// PROPÓSITO:
// Liberar la memoria de todos los nodos para evitar memory leaks.
//
// PROCESO:
// Recorrer la lista y hacer delete de cada nodo.
// ============================================================================

ListaSimple::~ListaSimple()
{
    Nodo *actual = cabeza;
    while (actual != nullptr)
    {
        Nodo *siguiente = actual->siguiente;
        delete actual; // Liberar memoria del nodo
        actual = siguiente;
    }
}

// ============================================================================
// INSERTAR_ORDENADO
//
// PROPÓSITO:
// Insertar un nuevo nodo manteniendo la lista ordenada por posición.
//
// POR QUÉ ORDENADA:
// Para facilitar la coalescencia (unir bloques adyacentes).
//
// EJEMPLO:
// Lista: [inicio:10, tam:5] → [inicio:50, tam:10] → NULL
// Insertar: [inicio:20, tam:3]
// Resultado: [10,5] → [20,3] → [50,10] → NULL
//                      ↑ insertado aquí
// ============================================================================

void ListaSimple::insertar_ordenado(int inicio, int tamanio)
{
    Nodo *nuevo = new Nodo(inicio, tamanio);

    // CASO 1: Lista vacía o insertar al inicio
    if (cabeza == nullptr || inicio < cabeza->inicio)
    {
        nuevo->siguiente = cabeza;
        cabeza = nuevo;
        return;
    }

    // CASO 2: Buscar posición de inserción
    Nodo *actual = cabeza;
    while (actual->siguiente != nullptr && actual->siguiente->inicio < inicio)
    {
        actual = actual->siguiente;
    }

    // Insertar entre actual y actual->siguiente
    nuevo->siguiente = actual->siguiente;
    actual->siguiente = nuevo;
}

// ============================================================================
// COALESCENCIA
//
// PROPÓSITO:
// Unir bloques libres adyacentes en un solo nodo más grande.
//
// POR QUÉ ES IMPORTANTE:
// Al liberar bloques, pueden quedar fragmentados. Unirlos hace que
// huecos grandes estén disponibles para allocaciones grandes.
//
// EJEMPLO:
// ANTES de coalescencia:
// [inicio:10, tam:5] → [inicio:15, tam:3] → [inicio:20, tam:2]
//  bloques 10-14       bloques 15-17        bloques 20-21
//                ↑ adyacentes ↑      ↑ adyacentes ↑
//
// DESPUÉS de coalescencia:
// [inicio:10, tam:8]
//  bloques 10-17 (unidos)
//
// ALGORITMO:
// Si nodo.inicio + nodo.tamanio == siguiente.inicio → ¡Son adyacentes!
// ============================================================================

void ListaSimple::coalescencia()
{
    if (cabeza == nullptr)
        return;

    Nodo *actual = cabeza;

    while (actual != nullptr && actual->siguiente != nullptr)
    {
        // Verificar si el siguiente nodo es adyacente
        if (actual->inicio + actual->tamanio == actual->siguiente->inicio)
        {
            // ¡Son adyacentes! Unir nodos
            Nodo *temp = actual->siguiente;
            actual->tamanio += temp->tamanio;    // Sumar tamaños
            actual->siguiente = temp->siguiente; // Saltar el nodo a eliminar
            delete temp;                         // Liberar memoria
            // No avanzar actual (puede haber más nodos adyacentes)
        }
        else
        {
            actual = actual->siguiente;
        }
    }
}

// ============================================================================
// BUSCAR_MEJOR_AJUSTE (Best Fit)
//
// PROPÓSITO:
// Encontrar el hueco más pequeño que sea suficiente.
//
// ESTRATEGIA BEST FIT:
// - Buscar en toda la lista
// - Encontrar el hueco que minimice el desperdicio
//
// EJEMPLO:
// Necesito: 10 bloques
// Huecos disponibles: [5], [15], [12], [100]
//                            ↑    ↑
//                          caben ambos
//
// Best Fit elige: [12] (desperdicio = 2)
// (No [15] porque desperdicia más: 5)
//
// ALTERNATIVAS:
// - First Fit: Primer hueco que cabe (más rápido pero más fragmentación)
// - Worst Fit: Hueco más grande (deja huecos grandes, pero puede agotar grandes)
// ============================================================================

ListaSimple::Nodo *ListaSimple::buscar_mejor_ajuste(int num_bloques)
{
    Nodo *mejor = nullptr;
    Nodo *actual = cabeza;
    int min_desperdicio = TOTAL_BLOQUES + 1;

    // Recorrer toda la lista
    while (actual != nullptr)
    {
        if (actual->tamanio >= num_bloques)
        { // ¿Cabe?
            int desperdicio = actual->tamanio - num_bloques;
            if (desperdicio < min_desperdicio)
            {
                min_desperdicio = desperdicio;
                mejor = actual;
            }
        }
        actual = actual->siguiente;
    }

    return mejor; // Puede ser nullptr si no encontró
}

// ============================================================================
// ALLOCAR
//
// PROPÓSITO:
// Ocupar N bloques del disco usando la lista.
//
// PROCESO:
// 1. Simular delay de I/O
// 2. Buscar mejor hueco (Best Fit)
// 3. Marcar bloques como ocupados en el disco real
// 4. Actualizar o eliminar el nodo
// ============================================================================

bool ListaSimple::allocar(int num_bloques)
{
    simular_acceso_disco(ALLOCACION, num_bloques);

    Nodo *nodo = buscar_mejor_ajuste(num_bloques);

    if (nodo == nullptr)
    {
        return false; // No hay espacio suficiente
    }

    // Marcar bloques como ocupados en el disco real
    for (int i = nodo->inicio; i < nodo->inicio + num_bloques; i++)
    {
        disco[i] = true;
    }

    bloques_ocupados += num_bloques;
    bloques_libres -= num_bloques;

    // ACTUALIZAR EL NODO:
    if (nodo->tamanio == num_bloques)
    {
        // El nodo se usa completo → eliminar de la lista
        if (nodo == cabeza)
        {
            cabeza = cabeza->siguiente;
            delete nodo;
        }
        else
        {
            // Buscar nodo anterior
            Nodo *anterior = cabeza;
            while (anterior->siguiente != nodo)
            {
                anterior = anterior->siguiente;
            }
            anterior->siguiente = nodo->siguiente;
            delete nodo;
        }
    }
    else
    {
        // El nodo se usa parcialmente → reducir tamaño
        nodo->inicio += num_bloques;  // Mover inicio
        nodo->tamanio -= num_bloques; // Reducir tamaño
    }

    return true;
}

// ============================================================================
// LIBERAR
//
// PROPÓSITO:
// Marcar bloques como libres y agregar a la lista.
//
// PROCESO:
// 1. Simular delay
// 2. Marcar bloques como libres en el disco
// 3. Insertar nuevo nodo en la lista
// 4. Coalescencia (unir bloques adyacentes)
// ============================================================================

bool ListaSimple::liberar(int inicio, int num_bloques)
{
    // Validación
    if (inicio < 0 || inicio + num_bloques > TOTAL_BLOQUES)
    {
        return false;
    }

    simular_acceso_disco(LIBERACION, num_bloques);

    // Marcar bloques como libres en el disco
    for (int i = inicio; i < inicio + num_bloques; i++)
    {
        if (disco[i])
        { // Solo si estaba ocupado
            disco[i] = false;
            bloques_ocupados--;
            bloques_libres++;
        }
    }

    // Insertar nuevo segmento libre en la lista
    insertar_ordenado(inicio, num_bloques);

    // IMPORTANTE: Coalescencia para unir bloques adyacentes
    coalescencia();

    return true;
}

// ============================================================================
// BUSCAR_BLOQUE_MAS_GRANDE
//
// PROPÓSITO:
// Encontrar el nodo con el tamaño más grande.
//
// VENTAJA DE LA LISTA:
// Solo recorre nodos de bloques libres (no todo el disco como bitmap).
// ============================================================================

int ListaSimple::buscar_bloque_mas_grande()
{
    simular_acceso_disco(BUSQUEDA);

    int max_tamanio = 0;
    Nodo *actual = cabeza;

    while (actual != nullptr)
    {
        max_tamanio = std::max(max_tamanio, actual->tamanio);
        actual = actual->siguiente;
    }

    return max_tamanio;
}

// ============================================================================
// IMPRIMIR_LISTA (para debugging)
//
// PROPÓSITO:
// Mostrar la lista de bloques libres.
// ============================================================================

void ListaSimple::imprimir_lista()
{
    std::cout << "Lista de bloques libres:\n";
    Nodo *actual = cabeza;
    int count = 0;

    if (actual == nullptr)
    {
        std::cout << "  (vacía - disco completamente ocupado)\n";
        return;
    }

    while (actual != nullptr)
    {
        std::cout << "  [" << actual->inicio << "-"
                  << (actual->inicio + actual->tamanio - 1)
                  << "] (" << actual->tamanio << " bloques)";

        actual = actual->siguiente;
        count++;

        if (actual != nullptr)
            std::cout << " → ";
        if (count % 4 == 0)
            std::cout << "\n  ";
    }
    std::cout << "\nTotal segmentos: " << count << "\n";
}