/*
 * LISTA_DOBLE.CPP
 *
 * IMPLEMENTA: Lista doblemente enlazada para gestión de disco
 *
 * DIFERENCIA CON LISTA SIMPLE:
 * Cada nodo tiene puntero al anterior Y al siguiente.
 *
 * EJEMPLO VISUAL:
 * NULL ← [nodo A] ↔ [nodo B] ↔ [nodo C] → NULL
 *  ↑       ↑ ↓        ↑ ↓        ↑ ↓      ↑
 *        prev next  prev next  prev next
 *
 * VENTAJAS:
 * 1. Recorrer en ambas direcciones
 * 2. Eliminar nodo es O(1) si tienes el puntero (no necesitas buscar el anterior)
 * 3. Insertar al final es O(1) (tenemos puntero cola)
 *
 * DESVENTAJAS:
 * 1. Más memoria (2 punteros por nodo vs 1)
 * 2. Más complejo de implementar
 */

#include "./core/disk_manager.h"
#include <iostream>
#include <algorithm>

// ============================================================================
// CONSTRUCTOR
//
// Similar a ListaSimple, pero mantenemos puntero a cola también.
// ============================================================================

ListaDoble::ListaDoble() : GestorDisco(), cabeza(nullptr), cola(nullptr)
{
    int inicio = -1;
    int tamanio = 0;

    for (int i = 0; i < TOTAL_BLOQUES; i++)
    {
        if (!disco[i])
        { // Libre
            if (inicio == -1)
            {
                inicio = i;
                tamanio = 1;
            }
            else
            {
                tamanio++;
            }
        }
        else
        { // Ocupado
            if (inicio != -1)
            {
                insertar_ordenado(inicio, tamanio);
                inicio = -1;
                tamanio = 0;
            }
        }
    }

    if (inicio != -1)
    {
        insertar_ordenado(inicio, tamanio);
    }
}

// ============================================================================
// DESTRUCTOR
// ============================================================================

ListaDoble::~ListaDoble()
{
    NodoDoble *actual = cabeza;
    while (actual != nullptr)
    {
        NodoDoble *siguiente = actual->siguiente;
        delete actual;
        actual = siguiente;
    }
}

// ============================================================================
// INSERTAR_ORDENADO
//
// CASOS ESPECIALES CON LISTA DOBLE:
// 1. Lista vacía → cabeza y cola apuntan al nuevo nodo
// 2. Insertar al inicio → actualizar cabeza->anterior
// 3. Insertar al final → actualizar cola y cola->siguiente
// 4. Insertar en medio → actualizar 4 punteros
// ============================================================================

void ListaDoble::insertar_ordenado(int inicio, int tamanio)
{
    NodoDoble *nuevo = new NodoDoble(inicio, tamanio);

    // CASO 1: Lista vacía
    if (cabeza == nullptr)
    {
        cabeza = nuevo;
        cola = nuevo;
        return;
    }

    // CASO 2: Insertar al inicio
    if (inicio < cabeza->inicio)
    {
        nuevo->siguiente = cabeza;
        cabeza->anterior = nuevo; // ← NUEVA conexión bidireccional
        cabeza = nuevo;
        return;
    }

    // CASO 3: Insertar al final
    if (inicio > cola->inicio)
    {
        nuevo->anterior = cola;
        cola->siguiente = nuevo;
        cola = nuevo;
        return;
    }

    // CASO 4: Insertar en medio
    NodoDoble *actual = cabeza;
    while (actual->siguiente != nullptr && actual->siguiente->inicio < inicio)
    {
        actual = actual->siguiente;
    }

    // Conectar 4 punteros:
    nuevo->siguiente = actual->siguiente;
    nuevo->anterior = actual;

    if (actual->siguiente != nullptr)
    {
        actual->siguiente->anterior = nuevo;
    }
    actual->siguiente = nuevo;
}

// ============================================================================
// COALESCENCIA
//
// Mismo algoritmo que lista simple, pero más eficiente porque
// eliminar_nodo es O(1) (no necesitamos buscar el anterior).
// ============================================================================

void ListaDoble::coalescencia()
{
    if (cabeza == nullptr)
        return;

    NodoDoble *actual = cabeza;

    while (actual != nullptr && actual->siguiente != nullptr)
    {
        if (actual->inicio + actual->tamanio == actual->siguiente->inicio)
        {
            // Adyacentes → unir
            NodoDoble *temp = actual->siguiente;
            actual->tamanio += temp->tamanio;
            actual->siguiente = temp->siguiente;

            if (temp->siguiente != nullptr)
            {
                temp->siguiente->anterior = actual;
            }
            else
            {
                cola = actual; // Actualizar cola si eliminamos el último
            }

            delete temp;
        }
        else
        {
            actual = actual->siguiente;
        }
    }
}

// ============================================================================
// ELIMINAR_NODO
//
// VENTAJA DE LISTA DOBLE:
// Eliminar un nodo es O(1) si tenemos el puntero.
// No necesitamos buscar el anterior (ya lo tenemos en nodo->anterior).
//
// CASOS:
// 1. Único nodo → cabeza = cola = nullptr
// 2. Eliminar cabeza → actualizar cabeza
// 3. Eliminar cola → actualizar cola
// 4. Eliminar medio → reconectar anterior y siguiente
// ============================================================================

void ListaDoble::eliminar_nodo(NodoDoble *nodo)
{
    if (nodo == nullptr)
        return;

    // CASO 1: Único nodo
    if (cabeza == nodo && cola == nodo)
    {
        cabeza = nullptr;
        cola = nullptr;
        delete nodo;
        return;
    }

    // CASO 2: Eliminar cabeza
    if (nodo == cabeza)
    {
        cabeza = cabeza->siguiente;
        if (cabeza != nullptr)
        {
            cabeza->anterior = nullptr;
        }
        delete nodo;
        return;
    }

    // CASO 3: Eliminar cola
    if (nodo == cola)
    {
        cola = cola->anterior;
        if (cola != nullptr)
        {
            cola->siguiente = nullptr;
        }
        delete nodo;
        return;
    }

    // CASO 4: Eliminar nodo intermedio
    nodo->anterior->siguiente = nodo->siguiente;
    nodo->siguiente->anterior = nodo->anterior;
    delete nodo;
}

// ============================================================================
// BUSCAR_MEJOR_AJUSTE
//
// Mismo algoritmo que lista simple (Best Fit).
// ============================================================================

ListaDoble::NodoDoble *ListaDoble::buscar_mejor_ajuste(int num_bloques)
{
    NodoDoble *mejor = nullptr;
    NodoDoble *actual = cabeza;
    int min_desperdicio = TOTAL_BLOQUES + 1;

    while (actual != nullptr)
    {
        if (actual->tamanio >= num_bloques)
        {
            int desperdicio = actual->tamanio - num_bloques;
            if (desperdicio < min_desperdicio)
            {
                min_desperdicio = desperdicio;
                mejor = actual;
            }
        }
        actual = actual->siguiente;
    }

    return mejor;
}

// ============================================================================
// ALLOCAR
//
// VENTAJA vs LISTA SIMPLE:
// Eliminar nodo es más eficiente gracias a eliminar_nodo() O(1).
// ============================================================================

bool ListaDoble::allocar(int num_bloques)
{
    simular_acceso_disco(ALLOCACION, num_bloques);

    NodoDoble *nodo = buscar_mejor_ajuste(num_bloques);

    if (nodo == nullptr)
    {
        return false;
    }

    // Marcar bloques en el disco
    for (int i = nodo->inicio; i < nodo->inicio + num_bloques; i++)
    {
        disco[i] = true;
    }

    bloques_ocupados += num_bloques;
    bloques_libres -= num_bloques;

    // Actualizar nodo
    if (nodo->tamanio == num_bloques)
    {
        // Eliminar nodo completo (O(1) gracias a lista doble)
        eliminar_nodo(nodo);
    }
    else
    {
        // Reducir tamaño
        nodo->inicio += num_bloques;
        nodo->tamanio -= num_bloques;
    }

    return true;
}

// ============================================================================
// LIBERAR
//
// Mismo proceso que lista simple.
// ============================================================================

bool ListaDoble::liberar(int inicio, int num_bloques)
{
    if (inicio < 0 || inicio + num_bloques > TOTAL_BLOQUES)
    {
        return false;
    }

    simular_acceso_disco(LIBERACION, num_bloques);

    // Marcar bloques como libres
    for (int i = inicio; i < inicio + num_bloques; i++)
    {
        if (disco[i])
        {
            disco[i] = false;
            bloques_ocupados--;
            bloques_libres++;
        }
    }

    // Insertar nuevo segmento
    insertar_ordenado(inicio, num_bloques);

    // Coalescencia
    coalescencia();

    return true;
}

// ============================================================================
// BUSCAR_BLOQUE_MAS_GRANDE
//
// Mismo algoritmo que lista simple.
//
// POSIBLE OPTIMIZACIÓN:
// Podríamos recorrer desde cola hacia atrás si los nodos más grandes
// tienden a estar al final. Pero para este proyecto, mantener simple.
// ============================================================================

int ListaDoble::buscar_bloque_mas_grande()
{
    simular_acceso_disco(BUSQUEDA);

    int max_tamanio = 0;
    NodoDoble *actual = cabeza;

    while (actual != nullptr)
    {
        max_tamanio = std::max(max_tamanio, actual->tamanio);
        actual = actual->siguiente;
    }

    return max_tamanio;
}

// ============================================================================
// IMPRIMIR_LISTA
//
// Mostrar con flechas bidireccionales.
// ============================================================================

void ListaDoble::imprimir_lista()
{
    std::cout << "Lista doble de bloques libres:\n";
    NodoDoble *actual = cabeza;
    int count = 0;

    if (actual == nullptr)
    {
        std::cout << "  (vacía - disco completamente ocupado)\n";
        return;
    }

    std::cout << "  NULL ← ";
    while (actual != nullptr)
    {
        std::cout << "[" << actual->inicio << "-"
                  << (actual->inicio + actual->tamanio - 1)
                  << "] (" << actual->tamanio << " bloques)";

        actual = actual->siguiente;
        count++;

        if (actual != nullptr)
            std::cout << " ↔ ";
        if (count % 3 == 0 && actual != nullptr)
            std::cout << "\n         ";
    }
    std::cout << " → NULL\n";
    std::cout << "Total segmentos: " << count << "\n";
}