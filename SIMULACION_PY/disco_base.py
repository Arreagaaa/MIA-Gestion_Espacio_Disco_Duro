import time
import random
import os
from config import *


class DiscoSimulado:
    """
    Representa un disco duro físico con 1024 bloques
    
    Simula los tiempos de acceso reales de un HDD (Hard Disk Drive)
    donde cada lectura/escritura toma 5ms debido a movimiento mecánico
    del cabezal.

    """
    
    def __init__(self):
        """
        Inicializa un disco vacío (todos los bloques libres)
        """
        self.TAMANIO = TAMAÑO_DISCO
        self.bloques = [False] * self.TAMANIO  # Todos libres al inicio
        
        # Estadísticas 
        self.accesos_lectura = 0
        self.accesos_escritura = 0
        
        if DEBUG:
            print(f"[DEBUG] Disco creado: {self.TAMANIO} bloques, todos libres")
    
    
    
    # OPERACIONES DE I/O (con delays)
    
    def leer_bloque(self, posicion):
        """
        Lee el estado de un bloque del disco (simula I/O)
        """
        if posicion < 0 or posicion >= self.TAMANIO:
            raise IndexError(f"Posición {posicion} fuera de rango [0-{self.TAMANIO-1}]")
        
        # SIMULAR DELAY DE LECTURA DE DISCO
        if SIMULAR_DELAY:
            time.sleep(DELAY_LECTURA)
        
        self.accesos_lectura += 1
        
        if DEBUG:
            estado = "ocupado" if self.bloques[posicion] else "libre"
            print(f"Lectura bloque {posicion}: {estado}")
        
        return self.bloques[posicion]
    
    
    def escribir_bloque(self, posicion, ocupado):
        """
        Escribe el estado de un bloque en el disco 
        """
        if posicion < 0 or posicion >= self.TAMANIO:
            raise IndexError(f"Posición {posicion} fuera de rango [0-{self.TAMANIO-1}]")
        
        # SIMULAR DELAY DE ESCRITURA DE DISCO
        if SIMULAR_DELAY:
            time.sleep(DELAY_ESCRITURA)
        
        self.accesos_escritura += 1
        
        if DEBUG:
            accion = "ocupar" if ocupado else "liberar"
            print(f"[DEBUG] Escritura bloque {posicion}: {accion}")
        
        self.bloques[posicion] = ocupado
    
    
    # LECTURA/ESCRITURA DIRECTA (sin delays)
    # Estas funciones NO simulan I/O, se usan solo para
    # operaciones internas de las estructuras de datos
    
    def leer_directo(self, posicion):
        """Lee sin simular delay (para uso interno)"""
        if posicion < 0 or posicion >= self.TAMANIO:
            raise IndexError(f"Posición {posicion} fuera de rango")
        return self.bloques[posicion]
    
    
    def escribir_directo(self, posicion, ocupado):
        """Escribe sin simular delay (para uso interno)"""
        if posicion < 0 or posicion >= self.TAMANIO:
            raise IndexError(f"Posición {posicion} fuera de rango")
        self.bloques[posicion] = ocupado
    
    
    
    # INICIALIZACIÓN DEL DISCO
    def inicializar_aleatorio(self, porcentaje_ocupado=PORCENTAJE_INICIAL_OCUPADO):
        """
        Llena el disco con un porcentaje de bloques ocupados aleatoriamente
        Esto simula un disco "usado" con fragmentación realista
        """
        cantidad_ocupar = int(self.TAMANIO * porcentaje_ocupado)
        
        # Crear lista de posiciones y mezclarlas aleatoriamente
        posiciones = list(range(self.TAMANIO))
        random.shuffle(posiciones)
        
        # Ocupar los primeros N bloques (ahora en orden aleatorio)
        for i in range(cantidad_ocupar):
            self.bloques[posiciones[i]] = True
        
        if DEBUG:
            libres = self.TAMANIO - cantidad_ocupar
            print(f"Disco inicializado:")
            print(f"        Ocupados: {cantidad_ocupar} bloques ({porcentaje_ocupado*100}%)")
            print(f"        Libres: {libres} bloques ({(1-porcentaje_ocupado)*100}%)")
    
   
    # PERSISTENCIA (guardar/cargar)
    
    def guardar_estado(self, archivo=ARCHIVO_DISCO):
        """
        Guarda el estado del disco en un archivo de texto
        
        Formato del archivo:
            1100101110...   (1024 caracteres, uno por bloque: 1=ocupado, 0=libre)
        """
        # Crear directorio si no existe
        directorio = os.path.dirname(archivo)
        if directorio and not os.path.exists(directorio):
            os.makedirs(directorio)
        
        # Convertir array de booleanos a string de 1s y 0s
        estado_str = ''.join('1' if bloque else '0' for bloque in self.bloques)
        
        with open(archivo, 'w') as f:
            f.write(estado_str)
        
        if DEBUG:
            print(f"Estado guardado en {archivo}")
    
    
    def cargar_estado(self, archivo=ARCHIVO_DISCO):
        """
        Carga el estado del disco desde un archivo de texto
        """
        if not os.path.exists(archivo):
            raise FileNotFoundError(f"No se encontro el archivo {archivo}")
        
        with open(archivo, 'r') as f:
            estado_str = f.read().strip()
        
        # Validar formato
        if len(estado_str) != self.TAMANIO:
            raise ValueError(f"El archivo debe contener exactamente {self.TAMANIO} caracteres")
        
        if not all(c in '01' for c in estado_str):
            raise ValueError("El archivo solo debe contener '0' y '1'")
        
        # Convertir string a array de booleanos
        self.bloques = [c == '1' for c in estado_str]
        
        # Resetear contadores
        self.accesos_lectura = 0
        self.accesos_escritura = 0
        
        if DEBUG:
            ocupados = sum(self.bloques)
            print(f"Estado cargado desde {archivo}")
            print(f"        Bloques ocupados: {ocupados}")
            print(f"        Bloques libres: {self.TAMANIO - ocupados}")
    
    
    
    # UTILIDADES
    
    
    def obtener_estadisticas(self):
        """
        Retorna estadísticas del disco
        
        Returns:
            dict: Diccionario con estadísticas
        """
        ocupados = sum(self.bloques)
        libres = self.TAMANIO - ocupados
        
        return {
            'total_bloques': self.TAMANIO,
            'bloques_ocupados': ocupados,
            'bloques_libres': libres,
            'porcentaje_ocupado': ocupados / self.TAMANIO,
            'accesos_lectura': self.accesos_lectura,
            'accesos_escritura': self.accesos_escritura,
            'accesos_totales': self.accesos_lectura + self.accesos_escritura
        }
    
    
    def visualizar_fragmentos(self, bloques_por_linea=64):
        """
        Muestra una visualización del estado del disco
        
        Args:
            bloques_por_linea (int): Cuántos bloques mostrar por línea
        """
        print("\n" + "="*bloques_por_linea)
        print("VISUALIZACIÓN DEL DISCO")
        print("■ = ocupado  □ = libre")
        print("="*bloques_por_linea)
        
        for i in range(0, self.TAMANIO, bloques_por_linea):
            linea = ""
            for j in range(bloques_por_linea):
                pos = i + j
                if pos < self.TAMANIO:
                    linea += "■" if self.bloques[pos] else "□"
            print(f"{i:4d}: {linea}")
        
        print("="*bloques_por_linea + "\n")
    
    
    def resetear_contadores(self):
        """Resetea los contadores de accesos a cero"""
        self.accesos_lectura = 0
        self.accesos_escritura = 0


