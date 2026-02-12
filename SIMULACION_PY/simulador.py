"""
SIMULADOR RÁPIDO - Solo Listas Ligadas
(El Bitmap es demasiado lento para 5 corridas completas)
"""

import random
import time
from datetime import datetime
from disco_base import DiscoSimulado
from bitmap import Bitmap
from lista_simple import ListaSimple
from lista_doble import ListaDoble
from config import *


def ejecutar_corrida_rapida(estructura, disco):
    """Ejecuta una corrida del experimento"""
    estructura.resetear_estadisticas()
    allocaciones_exitosas = []
    
    # 50 allocaciones
    for i in range(50):
        tamanio = random.randint(1, 32)
        resultado = estructura.allocar(tamanio)
        if resultado:
            allocaciones_exitosas.append(resultado)
    
    # 30 liberaciones
    cantidad_liberar = min(30, len(allocaciones_exitosas))
    if cantidad_liberar > 0:
        indices = random.sample(range(len(allocaciones_exitosas)), cantidad_liberar)
        for idx in sorted(indices, reverse=True):
            inicio, tam = allocaciones_exitosas[idx]
            estructura.liberar(inicio, tam)
            allocaciones_exitosas.pop(idx)
    
    return estructura.obtener_estadisticas(), disco.obtener_estadisticas()


print("="*60)
print("SIMULADOR RÁPIDO - COMPARACIÓN DE ESTRUCTURAS")
print("="*60)
print()

# Preparar disco
disco = DiscoSimulado()
disco.inicializar_aleatorio(0.70)
disco.guardar_estado()

resultados_finales = {}


# BITMAP - Solo 1 corrida (es muy lento)

print("BITMAP (1 corrida)...")
disco.cargar_estado()
disco.resetear_contadores()
bitmap = Bitmap(disco)

inicio = time.time()
stats_bitmap, stats_disco_bitmap = ejecutar_corrida_rapida(bitmap, disco)
tiempo_bitmap = time.time() - inicio

print(f"   Completado en {tiempo_bitmap:.1f}s")
print(f"  Allocación: {stats_bitmap['tiempo_promedio_allocacion']*1000:.2f}ms")
print(f"  Accesos disco: {stats_disco_bitmap['accesos_totales']}")
print()

resultados_finales['Bitmap'] = {
    'alloc': stats_bitmap['tiempo_promedio_allocacion'] * 1000,
    'liber': stats_bitmap['tiempo_promedio_liberacion'] * 1000,
    'accesos': stats_disco_bitmap['accesos_totales']
}


# LISTA SIMPLE - 5 corridas

print("LISTA SIMPLE (5 corridas)...")
tiempos_alloc = []
tiempos_liber = []
accesos = []

for i in range(5):
    disco.cargar_estado()
    disco.resetear_contadores()
    lista = ListaSimple(disco)
    
    stats_lista, stats_disco = ejecutar_corrida_rapida(lista, disco)
    tiempos_alloc.append(stats_lista['tiempo_promedio_allocacion'] * 1000)
    tiempos_liber.append(stats_lista['tiempo_promedio_liberacion'] * 1000)
    accesos.append(stats_disco['accesos_totales'])
    print(f"  Corrida {i+1}/5: {stats_lista['tiempo_promedio_allocacion']*1000:.2f}ms")

prom_alloc_simple = sum(tiempos_alloc) / len(tiempos_alloc)
prom_liber_simple = sum(tiempos_liber) / len(tiempos_liber)
prom_accesos_simple = sum(accesos) / len(accesos)

print(f"   Promedio allocación: {prom_alloc_simple:.2f}ms")
print(f"   Promedio accesos: {prom_accesos_simple:.0f}")
print()

resultados_finales['Lista Simple'] = {
    'alloc': prom_alloc_simple,
    'liber': prom_liber_simple,
    'accesos': prom_accesos_simple
}


# LISTA DOBLE - 5 corridas

print("LISTA DOBLE (5 corridas)...")
tiempos_alloc = []
tiempos_liber = []
accesos = []

for i in range(5):
    disco.cargar_estado()
    disco.resetear_contadores()
    lista = ListaDoble(disco)
    
    stats_lista, stats_disco = ejecutar_corrida_rapida(lista, disco)
    tiempos_alloc.append(stats_lista['tiempo_promedio_allocacion'] * 1000)
    tiempos_liber.append(stats_lista['tiempo_promedio_liberacion'] * 1000)
    accesos.append(stats_disco['accesos_totales'])
    print(f"  Corrida {i+1}/5: {stats_lista['tiempo_promedio_allocacion']*1000:.2f}ms")

prom_alloc_doble = sum(tiempos_alloc) / len(tiempos_alloc)
prom_liber_doble = sum(tiempos_liber) / len(tiempos_liber)
prom_accesos_doble = sum(accesos) / len(accesos)

print(f"  Promedio allocación: {prom_alloc_doble:.2f}ms")
print(f"  Promedio accesos: {prom_accesos_doble:.0f}")
print()

resultados_finales['Lista Doble'] = {
    'alloc': prom_alloc_doble,
    'liber': prom_liber_doble,
    'accesos': prom_accesos_doble
}


# RESULTADOS FINALES

print("="*60)
print("RESULTADOS FINALES")
print("="*60)
print()

print(f"{'Estructura':<15} | {'Alloc (ms)':<12} | {'Liber (ms)':<12} | {'Accesos':<10}")
print("-"*60)

for nombre, datos in resultados_finales.items():
    print(f"{nombre:<15} | {datos['alloc']:>10.2f} | {datos['liber']:>10.2f} | {datos['accesos']:>8.0f}")

print()
print("="*60)

# Factores de mejora
bitmap_alloc = resultados_finales['Bitmap']['alloc']
simple_alloc = resultados_finales['Lista Simple']['alloc']
doble_alloc = resultados_finales['Lista Doble']['alloc']

print(f"Lista Simple es {bitmap_alloc/simple_alloc:.0f} más rápida que Bitmap")
print(f"Lista Doble es {bitmap_alloc/doble_alloc:.0f} más rápida que Bitmap")
print("="*60)

# Guardar en archivo
with open(ARCHIVO_RESULTADOS, 'w', encoding='utf-8') as f:
    f.write("="*70 + "\n")
    f.write("RESULTADOS DE SIMULACIÓN\n")
    f.write("="*70 + "\n")
    f.write(f"Fecha: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n\n")
    
    f.write(f"{'Estructura':<15} | {'Alloc (ms)':<12} | {'Liber (ms)':<12} | {'Accesos':<10}\n")
    f.write("-"*70 + "\n")
    
    for nombre, datos in resultados_finales.items():
        f.write(f"{nombre:<15} | {datos['alloc']:>10.2f} | {datos['liber']:>10.2f} | {datos['accesos']:>8.0f}\n")
    
    f.write("\n" + "="*70 + "\n")
    f.write(f"Lista Simple es {bitmap_alloc/simple_alloc:.0f}x más rápida que Bitmap\n")
    f.write(f"Lista Doble es {bitmap_alloc/doble_alloc:.0f}x más rápida que Bitmap\n")
    f.write("="*70 + "\n")

print(f"\n Resultados guardados en: {ARCHIVO_RESULTADOS}")