/*
 * disk_manager_base.cpp
 *
 * Implementación de la clase base `GestorDisco` y del `MapaDeBits`.
 * Contiene los métodos comunes a las estructuras y la implementación
 * del mapa de bits.
 */

#include "disk_manager.h"
#include <iostream>
#include <fstream>
#include <random>
#include <thread>
#include <algorithm>

// Implementación de GestorDisco (clase base)

/*
 * CONSTRUCTOR
 *
 * QUÉ HACE:
 * - Crea el disco con 1024 bloques (todos libres al inicio)
 * - Inicializa contadores
 */
GestorDisco::GestorDisco()
    : disco(TOTAL_BLOQUES, false), // Crear array de 1024, todos en false (libres)
      bloques_libres(TOTAL_BLOQUES),
      bloques_ocupados(0)
{
    // Nada más que hacer aquí
}

/*
 * SIMULAR_ACCESO_DISCO
 *
 * PROPÓSITO:
 * Simular que el disco físico tarda tiempo en leer/escribir.
 *
 * POR QUÉ:
 * En un disco real (HDD):
 * - Mover el cabezal: 3-5ms (seek time)
 * - Esperar rotación: 2-4ms
 * - Leer/escribir: 1ms
 * Total: ~5-10ms por operación
 *
 * En tu computadora moderna todo pasa en nanosegundos,
 * entonces "dormimos" el programa para simular delays reales.
 */
void GestorDisco::simular_acceso_disco(TipoOperacion tipo, int num_bloques)
{
    int delay_ms = 0;

    switch (tipo)
    {
    case ALLOCACION:
        // Allocar tarda más (buscar + escribir)
        delay_ms = 5 * num_bloques; // 5ms por bloque
        break;

    case LIBERACION:
        // Liberar tarda menos (solo marcar como libre)
        delay_ms = 2 * num_bloques; // 2ms por bloque
        break;

    case BUSQUEDA:
        // Buscar: escanear estructura
        delay_ms = 1;
        break;
    }

    // DORMIR el programa por delay_ms milisegundos
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
}

/*
 * INICIALIZAR_DISCO
 *
 * PROPÓSITO:
 * Llenar el disco aleatoriamente hasta alcanzar el porcentaje deseado.
 *
 * EJEMPLO:
 * Si porcentaje_ocupado = 0.70 (70%):
 * - Debe ocupar 716 bloques (70% de 1024)
 * - Deja 308 bloques libres (30%)
 *
 * PROCESO:
 * 1. Calcular cuántos bloques ocupar
 * 2. Seleccionar bloques aleatorios
 * 3. Marcarlos como ocupados
 */
void GestorDisco::inicializar_disco(float porcentaje_ocupado)
{
    // Generador de números aleatorios
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 99);

    int umbral = static_cast<int>(porcentaje_ocupado * 100);
    bloques_libres = 0;
    bloques_ocupados = 0;

    // Para cada bloque, decidir si está ocupado
    for (int i = 0; i < TOTAL_BLOQUES; i++)
    {
        // Generar número aleatorio 0-99
        // Si es menor que umbral (70), marcar ocupado
        disco[i] = (dist(gen) < umbral);

        if (disco[i])
        {
            bloques_ocupados++;
        }
        else
        {
            bloques_libres++;
        }
    }

    std::cout << "Disco inicializado: " << bloques_ocupados << " bloques ocupados ("
              << (bloques_ocupados * 100.0 / TOTAL_BLOQUES) << "%)\n";
}

/*
 * GUARDAR_ESTADO
 *
 * PROPÓSITO:
 * Guardar el estado del disco en un archivo de texto.
 *
 * FORMATO:
 * # Comentario
 * 1 1 0 0 0 1 1 0 0 1 ...
 * (64 bloques por línea)
 */
void GestorDisco::guardar_estado(const std::string &archivo)
{
    std::ofstream file(archivo);

    if (!file.is_open())
    {
        std::cerr << "Error al abrir archivo para guardar: " << archivo << "\n";
        return;
    }

    file << "# Estado del disco (1 = ocupado, 0 = libre)\n";
    for (int i = 0; i < TOTAL_BLOQUES; i++)
    {
        file << (disco[i] ? "1" : "0");
        if ((i + 1) % 64 == 0)
            file << "\n"; // Nueva línea cada 64 bloques
        else
            file << " ";
    }

    file.close();
    std::cout << "Estado guardado en: " << archivo << "\n";
}

/*
 * CARGAR_ESTADO
 *
 * PROPÓSITO:
 * Leer el estado del disco desde un archivo.
 */
void GestorDisco::cargar_estado(const std::string &archivo)
{
    std::ifstream file(archivo);

    if (!file.is_open())
    {
        std::cerr << "Error al abrir archivo para cargar: " << archivo << "\n";
        return;
    }

    std::string linea;
    int index = 0;
    bloques_libres = 0;
    bloques_ocupados = 0;

    while (std::getline(file, linea) && index < TOTAL_BLOQUES)
    {
        if (linea[0] == '#')
            continue; // Saltar comentarios

        for (char c : linea)
        {
            if (c == '0' || c == '1')
            {
                disco[index] = (c == '1');
                if (disco[index])
                    bloques_ocupados++;
                else
                    bloques_libres++;
                index++;
                if (index >= TOTAL_BLOQUES)
                    break;
            }
        }
    }

    file.close();
    std::cout << "Estado cargado desde: " << archivo << "\n";
}

/*
 * GET_FRAGMENTACION
 *
 * PROPÓSITO:
 * Calcular el porcentaje de fragmentación externa.
 *
 * FÓRMULA:
 * fragmentación = (bloques_libres - bloque_más_grande) / bloques_libres * 100
 *
 * EJEMPLO:
 * - Bloques libres totales: 300
 * - Bloque más grande: 200
 * - Fragmentación: (300 - 200) / 300 * 100 = 33.3%
 *
 * INTERPRETACIÓN:
 * 33.3% del espacio libre está fragmentado (en trozos pequeños).
 */
float GestorDisco::get_fragmentacion() const
{
    if (bloques_libres == 0)
        return 0.0;

    // Encontrar el bloque libre más grande
    int max_consecutivos = 0;
    int consecutivos_actuales = 0;

    for (int i = 0; i < TOTAL_BLOQUES; i++)
    {
        if (!disco[i])
        { // Bloque libre
            consecutivos_actuales++;
            max_consecutivos = std::max(max_consecutivos, consecutivos_actuales);
        }
        else
        {
            consecutivos_actuales = 0;
        }
    }

    // Calcular fragmentación
    return (bloques_libres - max_consecutivos) * 100.0 / bloques_libres;
}

/*
 * CRONOMETRAJE
 *
 * PROPÓSITO:
 * Medir cuánto tarda una operación.
 */
void GestorDisco::iniciar_cronometro()
{
    tiempo_inicio = std::chrono::high_resolution_clock::now();
}

long long GestorDisco::detener_cronometro()
{
    auto tiempo_fin = std::chrono::high_resolution_clock::now();
    auto duracion = std::chrono::duration_cast<std::chrono::milliseconds>(
        tiempo_fin - tiempo_inicio);
    return duracion.count();
}

// IMPLEMENTACIÓN DE MapaDeBits

/*
 * CONSTRUCTOR
 *
 * QUÉ HACE:
 * - Copia el estado del disco al bitmap
 */
MapaDeBits::MapaDeBits() : GestorDisco()
{
    bitmap = disco; // Copiar estado inicial
}

/*
 * BUSCAR_BLOQUES_CONSECUTIVOS
 *
 * PROPÓSITO:
 * Buscar N bloques libres CONSECUTIVOS (uno tras otro).
 *
 * ALGORITMO:
 * 1. Recorrer el bitmap
 * 2. Contar bloques libres consecutivos
 * 3. Si encuentras N seguidos, retornar posición de inicio
 *
 * COMPLEJIDAD: O(n) - debe escanear todo en el peor caso
 *
 * Ejemplo:
 * bitmap: [1][1][0][0][0][1][0][0][0][0]
 *          ^^^^^  ^^^^^
 *          3 libres   4 libres
 *
 * buscar_bloques_consecutivos(3) -> retorna 2 (posición de inicio)
 */
int MapaDeBits::buscar_bloques_consecutivos(int num_bloques)
{
    int consecutivos = 0;

    for (int i = 0; i < TOTAL_BLOQUES; i++)
    {
        if (!bitmap[i])
        { // Bloque libre
            consecutivos++;
            if (consecutivos == num_bloques)
            {
                // ¡Encontrado! Retornar posición de inicio
                return i - num_bloques + 1;
            }
        }
        else
        {
            // Bloque ocupado, reiniciar contador
            consecutivos = 0;
        }
    }

    return -1; // No encontró suficientes bloques consecutivos
}

/*
 * ALLOCAR
 *
 * PROPÓSITO:
 * Ocupar (llenar) N bloques del disco.
 *
 * PROCESO:
 * 1. Simular delay de I/O
 * 2. Buscar N bloques libres consecutivos
 * 3. Marcarlos como ocupados
 * 4. Actualizar contadores
 */
int MapaDeBits::allocar(int num_bloques)
{
    simular_acceso_disco(ALLOCACION, num_bloques);

    int inicio = buscar_bloques_consecutivos(num_bloques);

    if (inicio == -1)
    {
        return -1; // No hay espacio suficiente
    }

    // Marcar bloques como ocupados
    for (int i = inicio; i < inicio + num_bloques; i++)
    {
        bitmap[i] = true;
        disco[i] = true;
    }

    bloques_ocupados += num_bloques;
    bloques_libres -= num_bloques;

    return inicio;
}

/*
 * LIBERAR
 *
 * PROPÓSITO:
 * Vaciar N bloques del disco (marcarlos libres).
 *
 * PROCESO:
 * 1. Validar parámetros
 * 2. Simular delay de I/O
 * 3. Marcar bloques como libres
 * 4. Actualizar contadores
 */
bool MapaDeBits::liberar(int inicio, int num_bloques)
{
    // Validación
    if (inicio < 0 || inicio + num_bloques > TOTAL_BLOQUES)
    {
        return false;
    }

    simular_acceso_disco(LIBERACION, num_bloques);

    // Marcar bloques como libres
    for (int i = inicio; i < inicio + num_bloques; i++)
    {
        if (bitmap[i])
        { // Solo si estaba ocupado
            bitmap[i] = false;
            disco[i] = false;
            bloques_ocupados--;
            bloques_libres++;
        }
    }

    return true;
}

/*
 * BUSCAR_BLOQUE_MAS_GRANDE
 *
 * PROPÓSITO:
 * Encontrar el segmento de bloques libres más grande.
 *
 * ALGORITMO:
 * Escanear todo el bitmap y encontrar la secuencia más larga de libres.
 */
int MapaDeBits::buscar_bloque_mas_grande()
{
    simular_acceso_disco(BUSQUEDA);

    int max_tamanio = 0;
    int tamanio_actual = 0;

    for (int i = 0; i < TOTAL_BLOQUES; i++)
    {
        if (!bitmap[i])
        {
            tamanio_actual++;
            max_tamanio = std::max(max_tamanio, tamanio_actual);
        }
        else
        {
            tamanio_actual = 0;
        }
    }

    return max_tamanio;
}

/*
 * IMPRIMIR_ESTADO
 *
 * PROPÓSITO:
 * Mostrar visualmente el estado del bitmap (para debugging).
 */
void MapaDeBits::imprimir_estado(int inicio, int fin)
{
    std::cout << "Estado del Bitmap [" << inicio << "-" << fin << "]:\n";
    for (int i = inicio; i <= fin && i < TOTAL_BLOQUES; i++)
    {
        std::cout << (bitmap[i] ? "█" : "░");
        if ((i - inicio + 1) % 64 == 0)
            std::cout << "\n";
    }
    std::cout << "\n█ = ocupado, ░ = libre\n";
}