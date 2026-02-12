/*
 * disk_manager.h
 *
 * Definiciones de la clase base `GestorDisco` y las interfaces para
 * las estructuras (MapaDeBits, ListaSimple, ListaDoble).
 */

#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H

#include <vector>
#include <string>
#include <chrono>

// Constantes del sistema

const int TOTAL_BLOQUES = 1024;       // Tamaño del disco: 1024 bloques
const int TAMANIO_BLOQUE = 1024;      // Cada bloque = 1KB
const float OCUPACION_INICIAL = 0.70; // 70% ocupado al inicio

// Enumeración: Tipos de operación (para simular delays)

enum TipoOperacion
{
    ALLOCACION, // Llenar bloques (5ms por bloque)
    LIBERACION, // Vaciar bloques (2ms por bloque)
    BUSQUEDA    // Buscar huecos (1ms total)
};

// Clase base abstracta: GestorDisco
// Define la interfaz común para los gestores de disco.

class GestorDisco
{
protected:
    // --------------------------------------------------------------------
    // ATRIBUTOS PROTEGIDOS (accesibles por clases hijas)
    // --------------------------------------------------------------------

    std::vector<bool> disco; // Estado REAL del disco
                             // disco[i] = true → bloque i ocupado
                             // disco[i] = false → bloque i libre

    int bloques_libres;   // Contador de bloques libres
    int bloques_ocupados; // Contador de bloques ocupados

    // Para medir tiempos
    std::chrono::high_resolution_clock::time_point tiempo_inicio;

    // --------------------------------------------------------------------
    // MÉTODO PROTEGIDO: Simular delays de I/O
    // --------------------------------------------------------------------
    void simular_acceso_disco(TipoOperacion tipo, int num_bloques = 1);

public:
    // --------------------------------------------------------------------
    // CONSTRUCTOR Y DESTRUCTOR
    // --------------------------------------------------------------------
    GestorDisco();
    virtual ~GestorDisco() {} // Virtual para que las hijas liberen memoria correctamente

    // --------------------------------------------------------------------
    // MÉTODOS VIRTUALES PUROS (= 0 significa "obligatorio implementar")
    //
    // EXPLICACIÓN:
    // Cada estructura implementará estos métodos a su manera:
    // - Bitmap: búsqueda secuencial en array
    // - Lista Simple: recorrido de nodos con un puntero
    // - Lista Doble: recorrido bidireccional
    // --------------------------------------------------------------------

    // Allocar: Llenar N bloques consecutivos
    // Retorna: true si éxito, false si no hay espacio
    virtual bool allocar(int num_bloques) = 0;

    // Liberar: Vaciar N bloques desde una posición
    // Retorna: true si éxito, false si error
    virtual bool liberar(int inicio, int num_bloques) = 0;

    // Buscar: Encontrar el bloque libre más grande
    // Retorna: Tamaño del bloque más grande
    virtual int buscar_bloque_mas_grande() = 0;

    // Obtener nombre de la estructura (para reportes)
    virtual std::string obtener_nombre() const = 0;

    // --------------------------------------------------------------------
    // MÉTODOS COMUNES (implementados en disk_manager_base.cpp)
    // --------------------------------------------------------------------

    void inicializar_disco(float porcentaje_ocupado);
    void guardar_estado(const std::string &archivo);
    void cargar_estado(const std::string &archivo);

    // Getters
    int get_bloques_libres() const { return bloques_libres; }
    int get_bloques_ocupados() const { return bloques_ocupados; }
    float get_fragmentacion() const;

    // Utilidades para cronometraje
    void iniciar_cronometro();
    long long detener_cronometro(); // Retorna milisegundos
};

// Clase: MapaDeBits
// Implementa la gestión usando un array booleano (bitmap).
// Ventaja: simple, acceso directo O(1).
// Desventaja: búsqueda lineal O(n) para bloques consecutivos.

class MapaDeBits : public GestorDisco
{
private:
    std::vector<bool> bitmap; // El mapa de bits

    // Buscar N bloques libres consecutivos
    // Retorna: Posición de inicio, o -1 si no encuentra
    int buscar_bloques_consecutivos(int num_bloques);

public:
    MapaDeBits();
    ~MapaDeBits() override {}

    // Implementación de métodos virtuales puros
    bool allocar(int num_bloques) override;
    bool liberar(int inicio, int num_bloques) override;
    int buscar_bloque_mas_grande() override;
    std::string obtener_nombre() const override { return "Mapa de Bits"; }

    // Método específico para debugging
    void imprimir_estado(int inicio = 0, int fin = 64);
};

// ============================================================================
// CLASE: ListaSimple
//
// IMPLEMENTA: Gestión usando lista simplemente enlazada
//
// CÓMO FUNCIONA:
// Solo guarda los BLOQUES LIBRES en nodos:
//
// [inicio:10, tam:5] → [inicio:20, tam:3] → [inicio:100, tam:50] → NULL
//      ↑                    ↑                      ↑
//   bloques 10-14       bloques 20-22         bloques 100-149
//      LIBRES              LIBRES                 LIBRES
//
// VENTAJAS:
// - Solo guarda huecos (eficiente si hay pocos huecos)
// - Si hay muchos bloques ocupados, usa menos memoria que bitmap
//
// DESVENTAJAS:
// - Búsqueda O(n) recorriendo nodos
// - Eliminar nodo requiere encontrar el anterior (O(n))
// ============================================================================

class ListaSimple : public GestorDisco
{
private:
    // Nodo de la lista
    struct Nodo
    {
        int inicio;      // Bloque de inicio del hueco
        int tamanio;     // Tamaño del hueco (cuántos bloques)
        Nodo *siguiente; // Puntero al siguiente hueco

        Nodo(int ini, int tam) : inicio(ini), tamanio(tam), siguiente(nullptr) {}
    };

    Nodo *cabeza; // Primer nodo de la lista

    // Métodos auxiliares privados
    void insertar_ordenado(int inicio, int tamanio);
    void coalescencia();                        // Unir bloques adyacentes
    Nodo *buscar_mejor_ajuste(int num_bloques); // Best Fit

public:
    ListaSimple();
    ~ListaSimple() override;

    bool allocar(int num_bloques) override;
    bool liberar(int inicio, int num_bloques) override;
    int buscar_bloque_mas_grande() override;
    std::string obtener_nombre() const override { return "Lista Simplemente Ligada"; }

    void imprimir_lista();
};

// ============================================================================
// CLASE: ListaDoble
//
// IMPLEMENTA: Gestión usando lista doblemente enlazada
//
// CÓMO FUNCIONA:
// Similar a lista simple, pero cada nodo tiene puntero al anterior también:
//
// NULL ← [nodo A] ↔ [nodo B] ↔ [nodo C] → NULL
//          ↑↓          ↑↓          ↑↓
//       adelante/   adelante/   adelante/
//        atrás       atrás       atrás
//
// VENTAJAS:
// - Recorrido bidireccional
// - Eliminar nodo es O(1) si tienes el puntero
//
// DESVENTAJAS:
// - Más memoria (2 punteros por nodo vs 1)
// - Implementación más compleja
// ============================================================================

class ListaDoble : public GestorDisco
{
private:
    // Nodo doblemente enlazado
    struct NodoDoble
    {
        int inicio;
        int tamanio;
        NodoDoble *siguiente;
        NodoDoble *anterior; // ← NUEVA: puntero hacia atrás

        NodoDoble(int ini, int tam)
            : inicio(ini), tamanio(tam), siguiente(nullptr), anterior(nullptr) {}
    };

    NodoDoble *cabeza;
    NodoDoble *cola; // Para inserción eficiente al final

    // Métodos auxiliares
    void insertar_ordenado(int inicio, int tamanio);
    void coalescencia();
    NodoDoble *buscar_mejor_ajuste(int num_bloques);
    void eliminar_nodo(NodoDoble *nodo);

public:
    ListaDoble();
    ~ListaDoble() override;

    bool allocar(int num_bloques) override;
    bool liberar(int inicio, int num_bloques) override;
    int buscar_bloque_mas_grande() override;
    std::string obtener_nombre() const override { return "Lista Doblemente Ligada"; }

    void imprimir_lista();
};

#endif // DISK_MANAGER_H