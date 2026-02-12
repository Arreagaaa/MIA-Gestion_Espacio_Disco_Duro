/*
 * lista_simple.cpp
 *
 * Implementa una lista simplemente enlazada para la gestión de bloques libres.
 * En lugar de guardar todos los bloques, se guardan solo los huecos libres
 * como nodos {inicio, tamanio}.
 */

#include "./core/disk_manager.h"
#include <iostream>
#include <algorithm>

// Constructor: construir la lista inicial de bloques libres a partir del disco

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

// Destructor: liberar memoria de los nodos

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

// insertar_ordenado: insertar un nuevo nodo manteniendo la lista ordenada

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

// coalescencia: unir bloques libres adyacentes en un único nodo

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

// buscar_mejor_ajuste (Best Fit): encontrar el hueco con menor desperdicio

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

int ListaSimple::allocar(int num_bloques)
{
    simular_acceso_disco(ALLOCACION, num_bloques);
    Nodo *nodo = buscar_mejor_ajuste(num_bloques);

    if (nodo == nullptr)
    {
        return -1; // No hay espacio suficiente
    }

    int inicio = nodo->inicio;

    // Marcar bloques como ocupados en el disco real
    for (int i = inicio; i < inicio + num_bloques; i++)
    {
        disco[i] = true;
    }

    bloques_ocupados += num_bloques;
    bloques_libres -= num_bloques;

    // Actualizar el nodo
    if (nodo->tamanio == num_bloques)
    {
        if (nodo == cabeza)
        {
            cabeza = cabeza->siguiente;
            delete nodo;
        }
        else
        {
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
        nodo->inicio += num_bloques;
        nodo->tamanio -= num_bloques;
    }

    return inicio;
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