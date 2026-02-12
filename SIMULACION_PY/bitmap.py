import time
from disco_base import DiscoSimulado


class Bitmap:
    """
    El bitmap usa directamente el array de bloques del disco porque cada posición del array
    """
    
    def __init__(self, disco):
        """
        Inicializa el bitmap con un disco
        """
        self.disco = disco
        self.nombre = "Bitmap"
        
        # Estadísticas para medición de tiempos
        self.tiempo_allocacion = []
        self.tiempo_liberacion = []
        self.tiempo_busqueda = []
    
    
    
    # OPERACIÓN 1: ALLOCAR BLOQUES
    
    def allocar(self, n_bloques):
        """
        Encuentra y reserva n bloques consecutivos libres
        
        ALGORITMO:
        1. Recorrer el array desde el inicio
        2. Cuando encuentro un bloque libre, inicio contador
        3. Si encuentro n bloques consecutivos libres, los marco como ocupados
        4. Si encuentro un bloque ocupado, reinicio contador
        """
        # Medir tiempo de ejecución
        tiempo_inicio = time.time()
        
        # Variables para el algoritmo de búsqueda
        posicion_actual = 0
        bloques_encontrados = 0
        posicion_inicio_vacio = -1
        
        # PASO 1: Buscar n bloques consecutivos libres
        while posicion_actual < self.disco.TAMANIO:
            
            # Leer estado del bloque actual (CON delay de I/O simulado)
            esta_ocupado = self.disco.leer_bloque(posicion_actual)
            
            if not esta_ocupado:  # Bloque está LIBRE
                
                if bloques_encontrados == 0:
                    # Primera vez que encuentro un bloque libre
                    posicion_inicio_vacio = posicion_actual
                
                bloques_encontrados += 1
                
                # ¿Ya encontré suficientes bloques consecutivos?
                if bloques_encontrados == n_bloques:
                    # ¡SÍ! Encontré n bloques libres consecutivos
                    break
                    
            else:  # Bloque está OCUPADO
                # Reiniciar contador porque la secuencia se interrumpió
                bloques_encontrados = 0
                posicion_inicio_vacio = -1
            
            posicion_actual += 1
        
        
        # PASO 2: Encontré suficiente espacio?
        if bloques_encontrados < n_bloques:
            # NO hay espacio suficiente
            tiempo_fin = time.time()
            self.tiempo_allocacion.append(tiempo_fin - tiempo_inicio)
            return None
        
        
        # PASO 3: Marcar los bloques como ocupados
        for i in range(n_bloques):
            posicion = posicion_inicio_vacio + i
            self.disco.escribir_bloque(posicion, True)  # Marcar como ocupado
        
        
        # Registrar tiempo y retornar resultado
        tiempo_fin = time.time()
        self.tiempo_allocacion.append(tiempo_fin - tiempo_inicio)
        
        return (posicion_inicio_vacio, n_bloques)
    
    
    
    # OPERACIÓN 2: LIBERAR BLOQUES
    
    
    def liberar(self, inicio, tamaño):
        """
        Marca bloques como libres (los devuelve al pool de espacio disponible)
        
        ALGORITMO:
        1. Desde la posición 'inicio'
        2. Marca 'tamaño' bloques consecutivos como libres
        3. Simple: no necesita reorganizar nada
        
        """
        tiempo_inicio = time.time()
        
        # Validar parámetros
        if inicio < 0 or inicio >= self.disco.TAMANIO:
            return False
        
        if inicio + tamaño > self.disco.TAMANIO:
            return False
        
        # Marcar cada bloque como libre
        for i in range(tamaño):
            posicion = inicio + i
            self.disco.escribir_bloque(posicion, False)  # Marcar como libre
        
        tiempo_fin = time.time()
        self.tiempo_liberacion.append(tiempo_fin - tiempo_inicio)
        
        return True
    
    
    
    # OPERACIÓN 3: BUSCAR ESPACIO MÁS GRANDE
    
    
    def buscar_mayor_espacio(self):
        """
        Encuentra el espacio libre más grande del disco
        
        ALGORITMO:
        1. Recorrer todo el disco
        2. Contar bloques libres consecutivos
        3. Recordar el espacio más grande encontrado
        
        """
        tiempo_inicio = time.time()
        
        # Variables para trackear el espacio más grande
        mayor_inicio = -1
        mayor_tamaño = 0
        
        # Variables para el espacio actual
        espacio_actual_inicio = -1
        espacio_actual_tamaño = 0
        
        # Recorrer todo el disco
        for posicion in range(self.disco.TAMANIO):
            
            esta_ocupado = self.disco.leer_bloque(posicion)
            
            if not esta_ocupado:  # Bloque LIBRE
                
                if espacio_actual_tamaño == 0:
                    # Inicio de un nuevo espacio
                    espacio_actual_inicio = posicion
                
                espacio_actual_tamaño += 1
                
            else:  # Bloque OCUPADO
                
                # Fin del espacio actual, ¿es el más grande hasta ahora?
                if espacio_actual_tamaño > mayor_tamaño:
                    mayor_tamaño = espacio_actual_tamaño
                    mayor_inicio = espacio_actual_inicio
                
                # Reiniciar espacio actual
                espacio_actual_tamaño = 0
                espacio_actual_inicio = -1
        
        # Verificar el último espacio (por si termina al final del disco)
        if espacio_actual_tamaño > mayor_tamaño:
            mayor_tamaño = espacio_actual_tamaño
            mayor_inicio = espacio_actual_inicio
        
        tiempo_fin = time.time()
        self.tiempo_busqueda.append(tiempo_fin - tiempo_inicio)
        
        # Se encontró algún espacio?
        if mayor_tamaño == 0:
            return None
        
        return (mayor_inicio, mayor_tamaño)
    
    
    
    # UTILIDADES Y ESTADÍSTICAS
    
    def obtener_estadisticas(self):
        """
        Retorna estadísticas de rendimiento
        """
        def promedio(lista):
            return sum(lista) / len(lista) if lista else 0
        
        return {
            'nombre': self.nombre,
            'allocaciones_realizadas': len(self.tiempo_allocacion),
            'liberaciones_realizadas': len(self.tiempo_liberacion),
            'busquedas_realizadas': len(self.tiempo_busqueda),
            'tiempo_promedio_allocacion': promedio(self.tiempo_allocacion),
            'tiempo_promedio_liberacion': promedio(self.tiempo_liberacion),
            'tiempo_promedio_busqueda': promedio(self.tiempo_busqueda),
        }
    
    
    def resetear_estadisticas(self):
        """Limpia las estadísticas de tiempo"""
        self.tiempo_allocacion = []
        self.tiempo_liberacion = []
        self.tiempo_busqueda = []
    
    
    def __str__(self):
        """Representación en string"""
        return f"Bitmap (gestiona {self.disco.TAMANIO} bloques)"

