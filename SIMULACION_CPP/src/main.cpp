/*
 * main.cpp
 *
 * Punto de entrada del programa.
 * Este archivo orquesta las corridas de la simulación, mide tiempos
 * y guarda los resultados en `data/resultados.txt`.
 */

#include "core/disk_manager.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <iomanip>
#include <memory>
#include <map>

// Estructura: ResultadoOperacion
// Guarda el resultado de una operación (allocar o liberar)

struct ResultadoOperacion
{
    long long tiempo_ms; // Tiempo que tardó en milisegundos
    bool exito;          // ¿Se completó exitosamente?
};

// Estructura: ResultadoEstructura
// Guarda los resultados de una estructura en una corrida

struct ResultadoEstructura
{
    std::string nombre;                        // "Mapa de Bits", etc.
    std::vector<long long> tiempos_allocacion; // Vector con 50 tiempos
    std::vector<long long> tiempos_liberacion; // Vector con 30 tiempos
    long long tiempo_busqueda;                 // Un solo tiempo
    float fragmentacion;                       // Porcentaje

    // Calcular promedio de allocaciones
    double promedio_allocacion() const
    {
        if (tiempos_allocacion.empty())
            return 0.0;
        long long suma = 0;
        for (auto t : tiempos_allocacion)
            suma += t;
        return static_cast<double>(suma) / tiempos_allocacion.size();
    }

    // Calcular promedio de liberaciones
    double promedio_liberacion() const
    {
        if (tiempos_liberacion.empty())
            return 0.0;
        long long suma = 0;
        for (auto t : tiempos_liberacion)
            suma += t;
        return static_cast<double>(suma) / tiempos_liberacion.size();
    }
};

// Función: ejecutar_secuencia_pruebas
// Ejecuta la secuencia completa de pruebas para una estructura.
// Proceso: 50 allocaciones, 30 liberaciones, 1 búsqueda, calcular fragmentación.

ResultadoEstructura ejecutar_secuencia_pruebas(GestorDisco *gestor)
{
    ResultadoEstructura resultado;
    resultado.nombre = gestor->obtener_nombre();

    // Generadores aleatorios
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist_tam(1, 32); // Tamaño 1-32
    std::uniform_int_distribution<> dist_bloque(0, TOTAL_BLOQUES - 1);

    // Para rastrear allocaciones exitosas (para liberar después)
    std::vector<std::pair<int, int>> allocaciones_exitosas; // {inicio, tamaño}

    std::cout << "  Ejecutando 50 allocaciones...\n";

    // Fase 1: 50 allocaciones
    for (int i = 0; i < 50; i++)
    {
        int num_bloques = dist_tam(gen); // Tamaño aleatorio 1-32

        // Medir tiempo
        gestor->iniciar_cronometro();
        bool exito = gestor->allocar(num_bloques);
        long long tiempo = gestor->detener_cronometro();

        // Guardar tiempo (solo si fue exitoso)
        if (exito)
        {
            resultado.tiempos_allocacion.push_back(tiempo);
            // Guardar para poder liberar después
            // (Aproximado - no sabemos exactamente dónde allocó)
            allocaciones_exitosas.push_back({dist_bloque(gen), num_bloques});
        }

        // Progreso cada 10 operaciones
        if ((i + 1) % 10 == 0)
        {
            std::cout << "    Allocación " << (i + 1) << "/50 completada\n";
        }
    }

    std::cout << "  Ejecutando 30 liberaciones...\n";

    // Fase 2: 30 liberaciones
    int liberaciones_realizadas = 0;
    for (int i = 0; i < 30 && !allocaciones_exitosas.empty(); i++)
    {
        // Seleccionar una allocación aleatoria para liberar
        std::uniform_int_distribution<> dist_alloc(0, allocaciones_exitosas.size() - 1);
        int index = dist_alloc(gen);

        auto [inicio, tamanio] = allocaciones_exitosas[index];

        // Medir tiempo
        gestor->iniciar_cronometro();
        bool exito = gestor->liberar(inicio, tamanio);
        long long tiempo = gestor->detener_cronometro();

        if (exito)
        {
            resultado.tiempos_liberacion.push_back(tiempo);
            // Remover de la lista (ya fue liberado)
            allocaciones_exitosas.erase(allocaciones_exitosas.begin() + index);
            liberaciones_realizadas++;
        }

        if ((i + 1) % 10 == 0)
        {
            std::cout << "    Liberación " << (i + 1) << "/30 completada\n";
        }
    }

    std::cout << "  Midiendo búsqueda del bloque más grande...\n";

    // Fase 3: búsqueda
    gestor->iniciar_cronometro();
    int bloque_mayor = gestor->buscar_bloque_mas_grande();
    resultado.tiempo_busqueda = gestor->detener_cronometro();

    std::cout << "    Bloque libre más grande: " << bloque_mayor << " bloques\n";

    // Fase 4: fragmentación
    resultado.fragmentacion = gestor->get_fragmentacion();

    return resultado;
}

// Función: guardar_resultados
// Escribir resultados de una corrida en resultados.txt

void guardar_resultados(const std::vector<ResultadoEstructura> &resultados,
                        int num_corrida)
{
    std::ofstream file("data/resultados.txt", std::ios::app);

    if (!file.is_open())
    {
        std::cerr << "Error al abrir archivo de resultados\n";
        return;
    }

    file << "\n----\n";
    file << "Corrida " << num_corrida << "\n";
    file << "----\n\n";

    for (const auto &res : resultados)
    {
        file << "Estructura: " << res.nombre << "\n";
        file << "  Allocación promedio: " << std::fixed << std::setprecision(2)
             << res.promedio_allocacion() << " ms\n";
        file << "  Liberación promedio: " << res.promedio_liberacion() << " ms\n";
        file << "  Búsqueda bloque grande: " << res.tiempo_busqueda << " ms\n";
        file << "  Fragmentación: " << res.fragmentacion << "%\n";
        file << "  Allocaciones exitosas: " << res.tiempos_allocacion.size() << "/50\n";
        file << "  Liberaciones exitosas: " << res.tiempos_liberacion.size() << "/30\n";
        file << "\n";
    }

    file.close();
}

// Función: imprimir_resumen_final
// Muestra la tabla comparativa con promedios de las corridas.

void imprimir_resumen_final(const std::vector<std::vector<ResultadoEstructura>> &todas_corridas)
{
    std::cout << "\n--- RESUMEN FINAL - " << todas_corridas.size() << " corridas ---\n\n";

    // Acumular datos por estructura
    std::map<std::string, std::vector<double>> promedios_alloc;
    std::map<std::string, std::vector<double>> promedios_lib;
    std::map<std::string, std::vector<long long>> tiempos_busq;
    std::map<std::string, std::vector<float>> fragmentaciones;

    // Recolectar datos de todas las corridas
    for (const auto &corrida : todas_corridas)
    {
        for (const auto &res : corrida)
        {
            promedios_alloc[res.nombre].push_back(res.promedio_allocacion());
            promedios_lib[res.nombre].push_back(res.promedio_liberacion());
            tiempos_busq[res.nombre].push_back(res.tiempo_busqueda);
            fragmentaciones[res.nombre].push_back(res.fragmentacion);
        }
    }

    // Imprimir tabla
    std::cout << std::left << std::setw(25) << "Estructura"
              << std::setw(15) << "Alloc (ms)"
              << std::setw(15) << "Liber (ms)"
              << std::setw(15) << "Búsq (ms)"
              << std::setw(12) << "Frag (%)\n";
    std::cout << std::string(82, '-') << "\n";

    for (const auto &[nombre, tiempos] : promedios_alloc)
    {
        // Calcular promedios de las 5 corridas
        double avg_alloc = 0, avg_lib = 0, avg_busq = 0, avg_frag = 0;

        for (double t : tiempos)
            avg_alloc += t;
        avg_alloc /= tiempos.size();

        for (double t : promedios_lib[nombre])
            avg_lib += t;
        avg_lib /= promedios_lib[nombre].size();

        for (long long t : tiempos_busq[nombre])
            avg_busq += t;
        avg_busq /= tiempos_busq[nombre].size();

        for (float f : fragmentaciones[nombre])
            avg_frag += f;
        avg_frag /= fragmentaciones[nombre].size();

        // Imprimir fila
        std::cout << std::left << std::setw(25) << nombre
                  << std::fixed << std::setprecision(2)
                  << std::setw(15) << avg_alloc
                  << std::setw(15) << avg_lib
                  << std::setw(15) << avg_busq
                  << std::setw(12) << avg_frag << "\n";
    }

    std::cout << "\n";
}

// Función principal

int main()
{
    std::cout << "Simulador de gestión de espacio en disco duro - Comparación de estructuras\n\n";

    const int NUM_CORRIDAS = 5;
    std::vector<std::vector<ResultadoEstructura>> todas_corridas;

    // Limpiar archivo de resultados previo
    std::ofstream file_clear("data/resultados.txt");
    file_clear << "RESULTADOS DE SIMULACIÓN\n";
    file_clear << "Fecha: " << __DATE__ << " " << __TIME__ << "\n";
    file_clear << "Configuración:\n";
    file_clear << "  - Total bloques: " << TOTAL_BLOQUES << "\n";
    file_clear << "  - Tamaño bloque: " << TAMANIO_BLOQUE << " bytes\n";
    file_clear << "  - Ocupación inicial: " << (OCUPACION_INICIAL * 100) << "%\n";
    file_clear << "  - Número de corridas: " << NUM_CORRIDAS << "\n";
    file_clear.close();

    // ========================================================================
    // LOOP PRINCIPAL: 5 CORRIDAS
    // ========================================================================
    for (int corrida = 1; corrida <= NUM_CORRIDAS; corrida++)
    {
        std::cout << "\nCorrida " << corrida << " de " << NUM_CORRIDAS << "\n\n";

        std::vector<ResultadoEstructura> resultados_corrida;

        // Crear las 3 estructuras
        std::vector<std::unique_ptr<GestorDisco>> gestores;
        gestores.push_back(std::make_unique<MapaDeBits>());
        gestores.push_back(std::make_unique<ListaSimple>());
        gestores.push_back(std::make_unique<ListaDoble>());

        // Inicializar todos con el mismo estado (70% ocupado)
        std::cout << "Inicializando disco (" << (OCUPACION_INICIAL * 100) << "% ocupado)...\n";
        for (auto &gestor : gestores)
        {
            gestor->inicializar_disco(OCUPACION_INICIAL);
        }

        // Guardar estado inicial (solo una vez)
        if (corrida == 1)
        {
            gestores[0]->guardar_estado("data/disco_inicial.txt");
        }

        // Ejecutar pruebas para cada estructura
        for (auto &gestor : gestores)
        {
            std::cout << "\n--- " << gestor->obtener_nombre() << " ---\n";

            ResultadoEstructura resultado = ejecutar_secuencia_pruebas(gestor.get());
            resultados_corrida.push_back(resultado);
        }

        // Guardar resultados de esta corrida
        guardar_resultados(resultados_corrida, corrida);
        todas_corridas.push_back(resultados_corrida);

        std::cout << "\nCorrida " << corrida << " completada\n";
    }

    // ========================================================================
    // RESUMEN FINAL
    // ========================================================================
    imprimir_resumen_final(todas_corridas);

    std::cout << "Resultados guardados en: data/resultados.txt\n";
    std::cout << "Estado inicial guardado en: data/disco_inicial.txt\n";
    std::cout << "\nSimulación completada exitosamente.\n\n";

    return 0;
}