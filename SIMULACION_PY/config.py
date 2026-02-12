"""Aqui van a estar todas las configuraciones globales"""


# Para el Disc
TAMAÑO_DISCO = 1024
TAMAÑO_BLOQUE_KB = 1
PORCENTAJE_INICIAL_OCUPADO = 0.70

# Tiempos de simulacion
DELAY_LECTURA_MS = 1 # Milisegundos por lectura de bloque
DELAY_ESCRITURA_MS = 1 # Milisegundos por escritura de bloque

# Convertir a segundos para time.sleep()
DELAY_LECTURA = DELAY_LECTURA_MS / 1000.0    # 0.005 segundos
DELAY_ESCRITURA = DELAY_ESCRITURA_MS / 1000.0

# Parametros 
CANTIDAD_CORRIDAS = 5       
CANTIDAD_ALLOCACIONES = 50   # 50 allocaciones 
CANTIDAD_LIBERACIONES = 30   # 30 liberaciones 

# Rango de tamaños para allocaciones aleatorias
TAMAÑO_MIN_ALLOCACION = 1    # Mínimo 1 bloque
TAMAÑO_MAX_ALLOCACION = 32   # Máximo 32 bloques

#RUTAS
RUTA_DATOS = "datos/"
RUTA_GRAFICOS = "graficos/"
ARCHIVO_DISCO = RUTA_DATOS + "disco.txt"
ARCHIVO_RESULTADOS = RUTA_DATOS + "resultados.txt"

#Para debug
DEBUG = False                
SIMULAR_DELAY = True 