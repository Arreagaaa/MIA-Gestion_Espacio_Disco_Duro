
import time
from disco_base import DiscoSimulado
from nodo import Nodo

"""
LISTA LIGADA SIMPLE - Estructura de gestión de espacio

La Lista Ligada Simple solo trackea los espacioS LIBRES del disco,
no los bloques ocupados. Esto la hace mucho más eficiente que el Bitmap
cuando el disco está muy ocupado.
"""

class ListaSimple:
    """
    Atributos:
        disco (DiscoSimulado): Referencia al disco simulado
        cabeza (NodoSimple): Primer nodo de la lista (None si vacía)
        nombre (str): Nombre de la estructura
    """
    
    def __init__(self, disco):

        self.disco = disco
        self.cabeza = None  # Lista vacía inicialmente
        self.nombre = "Lista Simple"
        
        # Estadísticas
        self.tiempo_allocacion = []
        self.tiempo_liberacion = []
        self.tiempo_busqueda = []
        
        #  Construye la lista inicial de espacios libres
        self._construir_lista_inicial()
    
    
    def _construir_lista_inicial(self):
        """
        Escanea el disco y construye la lista de espacios libres
        
        Este método se ejecuta UNA VEZ al inicializar la estructura.
        Recorre el disco y crea un nodo por cada grupo de bloques
        libres consecutivos.
        """
        espacio_inicio = -1
        espacio_tamaño = 0
        
        # Recorrer todo el disco
        for pos in range(self.disco.TAMANIO):
            esta_ocupado = self.disco.leer_directo(pos)  # Sin delay (interno)
            
            if not esta_ocupado:  # Bloque LIBRE
                if espacio_tamaño == 0:
                    espacio_inicio = pos
                espacio_tamaño += 1
                
            else:  # Bloque OCUPADO
                if espacio_tamaño > 0:
                    # Termina un espacio, crear nodo
                    self._insertar_nodo_ordenado(espacio_inicio, espacio_tamaño)
                    espacio_tamaño = 0
        
        # Último espacio (si el disco termina con bloques libres)
        if espacio_tamaño > 0:
            self._insertar_nodo_ordenado(espacio_inicio, espacio_tamaño)
    
    
    def _insertar_nodo_ordenado(self, inicio, tamaño):
        """
        Inserta un nuevo nodo en la lista manteniéndola ordenada por posición
        
        La lista SIEMPRE está ordenada de menor a mayor posición.
        Esto facilita encontrar espacios adyacentes para fusionar.
        """
        nuevo_nodo = Nodo(inicio, tamaño)
        
        # CASO 1: Lista vacía
        if self.cabeza is None:
            self.cabeza = nuevo_nodo
            return
        
        # CASO 2: Insertar al inicio (antes de cabeza)
        if inicio < self.cabeza.inicio:
            nuevo_nodo.siguiente = self.cabeza
            self.cabeza = nuevo_nodo
            return
        
        # CASO 3: Insertar en medio o al final
        actual = self.cabeza
        while actual.siguiente is not None and actual.siguiente.inicio < inicio:
            actual = actual.siguiente
        
        # Insertar después de 'actual'
        nuevo_nodo.siguiente = actual.siguiente
        actual.siguiente = nuevo_nodo
    
    
    
    # OPERACIÓN 1: ALLOCAR BLOQUES
    
    def allocar(self, n_bloques):
        """
        Encuentra y reserva n bloques consecutivos libres
        
        ALGORITMO:
        1. Recorrer la lista de nodos (NO el disco completo)
        2. Buscar el primer nodo con tamaño >= n_bloques
        3. Usar ese nodo (total o parcialmente)
        4. Actualizar la lista
        """
        tiempo_inicio = time.time()
        
        # CASO ESPECIAL: Lista vacía
        if self.cabeza is None:
            tiempo_fin = time.time()
            self.tiempo_allocacion.append(tiempo_fin - tiempo_inicio)
            return None
        
        # Buscar nodo adecuado
        anterior = None
        actual = self.cabeza
        
        while actual is not None:
            
            # ¿Este nodo tiene suficiente espacio?
            if actual.tamaño >= n_bloques:
                
                # ENCONTRADO! Usar este nodo
                inicio_allocacion = actual.inicio
                
                # CASO A: Usar TODO el nodo (tamaño exacto o sobra poco)
                if actual.tamaño == n_bloques:
                    # Eliminar el nodo de la lista
                    if anterior is None:
                        # Es el primer nodo
                        self.cabeza = actual.siguiente
                    else:
                        # Es un nodo intermedio/final
                        anterior.siguiente = actual.siguiente
                
                # CASO B: Usar PARTE del nodo (sobra espacio)
                else:
                    # Reducir el nodo
                    actual.inicio += n_bloques
                    actual.tamaño -= n_bloques
                
                # Marcar bloques como ocupados en el disco
                for i in range(n_bloques):
                    self.disco.escribir_bloque(inicio_allocacion + i, True)
                
                tiempo_fin = time.time()
                self.tiempo_allocacion.append(tiempo_fin - tiempo_inicio)
                return (inicio_allocacion, n_bloques)
            
            # Siguiente nodo
            anterior = actual
            actual = actual.siguiente
        
        # No se encontró espacio suficiente
        tiempo_fin = time.time()
        self.tiempo_allocacion.append(tiempo_fin - tiempo_inicio)
        return None
    
    
    
    # OPERACIÓN 2: LIBERAR BLOQUES
    
    
    def liberar(self, inicio, tamaño):
        """
        Marca bloques como libres y los agrega a la lista
        
        ALGORITMO:
        1. Marcar bloques como libres en el disco
        2. Crear/actualizar nodo en la lista
        3. FUSIONAR con nodos adyacentes si es posible
        
        La fusión es importante para evitar fragmentación de la lista.
        """
        tiempo_inicio = time.time()
        
        # Validar parámetros
        if inicio < 0 or inicio >= self.disco.TAMANIO:
            return False
        if inicio + tamaño > self.disco.TAMANIO:
            return False
        
        # Marcar bloques como libres en el disco
        for i in range(tamaño):
            self.disco.escribir_bloque(inicio + i, False)
        
        # Agregar/fusionar en la lista
        self._fusionar_espacio(inicio, tamaño)
        
        tiempo_fin = time.time()
        self.tiempo_liberacion.append(tiempo_fin - tiempo_inicio)
        return True
    
    
    def _fusionar_espacio(self, inicio, tamaño):
        """
        Inserta un espacio libre fusionándolo con nodos adyacentes si es posible
        
        Casos de fusión:
        1. Fusión con nodo anterior: [Nodo][espacioNuevo] -> [NodoExpandido]
        2. Fusión con nodo siguiente: [espacioNuevo][Nodo] -> [NodoExpandido]
        3. Fusión con ambos: [Nodo1][espacioNuevo][Nodo2] -> [NodoExpandido]
        4. Sin fusión: Insertar como nodo nuevo
        """
        fin = inicio + tamaño - 1  # Último bloque del espacio
        
        # Buscar nodos adyacentes
        anterior = None
        actual = self.cabeza
        
        fusionado_anterior = False
        fusionado_siguiente = False
        
        while actual is not None:
            
            # FUSIÓN CON NODO ANTERIOR
            # El espacio nuevo está justo después de este nodo?
            if actual.inicio + actual.tamaño == inicio:
                # Expandir el nodo actual
                actual.tamaño += tamaño
                fusionado_anterior = True
                
                # También se puede fusionar con el siguiente?
                if actual.siguiente is not None:
                    if actual.inicio + actual.tamaño == actual.siguiente.inicio:
                        # Fusionar con siguiente también
                        actual.tamaño += actual.siguiente.tamaño
                        actual.siguiente = actual.siguiente.siguiente
                
                break
            
            # FUSIÓN CON NODO SIGUIENTE
            # El espacio nuevo está justo antes de este nodo?
            if fin + 1 == actual.inicio:
                # Expandir el nodo actual hacia atrás
                actual.inicio = inicio
                actual.tamaño += tamaño
                fusionado_siguiente = True
                break
            
            # Avanzar
            anterior = actual
            actual = actual.siguiente
        
        # Si no se fusionó con ninguno, insertar como nodo nuevo
        if not fusionado_anterior and not fusionado_siguiente:
            self._insertar_nodo_ordenado(inicio, tamaño)
    
    
    
    # OPERACIÓN 3: BUSCAR espacio MÁS GRANDE
    def buscar_mayor_espacio(self):
        """
        Encuentra el espacio libre más grande
        
        ALGORITMO:
        1. Recorrer la lista de nodos (NO el disco)
        2. Trackear el nodo con mayor tamaño
        """
        tiempo_inicio = time.time()
        
        if self.cabeza is None:
            tiempo_fin = time.time()
            self.tiempo_busqueda.append(tiempo_fin - tiempo_inicio)
            return None
        
        # Recorrer nodos buscando el más grande
        mayor_nodo = self.cabeza
        actual = self.cabeza.siguiente
        
        while actual is not None:
            if actual.tamaño > mayor_nodo.tamaño:
                mayor_nodo = actual
            actual = actual.siguiente
        
        tiempo_fin = time.time()
        self.tiempo_busqueda.append(tiempo_fin - tiempo_inicio)
        
        return (mayor_nodo.inicio, mayor_nodo.tamaño)
    
    
    
    # UTILIDADES    
    def obtener_estadisticas(self):
        """Retorna estadísticas de rendimiento"""
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
        """Limpia estadísticas"""
        self.tiempo_allocacion = []
        self.tiempo_liberacion = []
        self.tiempo_busqueda = []
    
    
    def contar_nodos(self):
        """Cuenta cuántos nodos hay en la lista"""
        count = 0
        actual = self.cabeza
        while actual is not None:
            count += 1
            actual = actual.siguiente
        return count
    
    
    def visualizar_lista(self):
        """Muestra la lista de espacios libres"""
        if self.cabeza is None:
            print("Lista vacía (sin espacios libres)")
            return
        
        print("Lista de espacios libres:")
        actual = self.cabeza
        nodos = []
        while actual is not None:
            nodos.append(f"[{actual.inicio}:{actual.inicio+actual.tamaño-1}]({actual.tamaño})")
            actual = actual.siguiente
        print(" → ".join(nodos) + " → NULL")
    
    
    def __str__(self):
        """Representación en string"""
        return f"Lista Simple ({self.contar_nodos()} espacios libres)"



# PRUEBAS DE LISTA SIMPLE
if __name__ == "__main__":
    print("=== PRUEBA DE LISTA SIMPLE ===\n")
    
    # Crear disco
    disco = DiscoSimulado()
    disco.inicializar_aleatorio(0.50)  # 50% ocupado
    
    # Crear lista simple
    print("Construyendo lista de espacios libres...")
    lista = ListaSimple(disco)
    print(f"✓ {lista}\n")
    
    # Visualizar lista inicial
    lista.visualizar_lista()
    print()
    
    # Estado del disco
    stats_disco = disco.obtener_estadisticas()
    print(f"Estado del disco:")
    print(f"  Ocupados: {stats_disco['bloques_ocupados']}")
    print(f"  Libres: {stats_disco['bloques_libres']}\n")
    
    # PRUEBA 1: Buscar espacio más grande
    print("--- PRUEBA 1: Buscar espacio más grande ---")
    espacio = lista.buscar_mayor_espacio()
    if espacio:
        print(f"✓ espacio más grande: {espacio[1]} bloques en posición {espacio[0]}\n")
    
    # PRUEBA 2: Allocar bloques
    print("--- PRUEBA 2: Allocar bloques ---")
    tamaños = [10, 5, 15]
    allocaciones = []
    
    for tam in tamaños:
        print(f"Allocando {tam} bloques...", end=" ")
        resultado = lista.allocar(tam)
        if resultado:
            allocaciones.append(resultado)
            print(f"✓ Posición {resultado[0]}")
        else:
            print("✗ Sin espacio")
    
    print(f"\n✓ Lista ahora tiene {lista.contar_nodos()} nodos\n")
    
    # PRUEBA 3: Liberar bloques
    if len(allocaciones) >= 2:
        print("--- PRUEBA 3: Liberar bloques ---")
        inicio, tam = allocaciones[0]
        print(f"Liberando {tam} bloques desde posición {inicio}...")
        lista.liberar(inicio, tam)
        print(f"✓ Lista ahora tiene {lista.contar_nodos()} nodos")
        lista.visualizar_lista()
        print()
    
    # Estadísticas
    print("=== ESTADÍSTICAS DE RENDIMIENTO ===")
    stats = lista.obtener_estadisticas()
    print(f"Allocaciones: {stats['allocaciones_realizadas']}")
    print(f"  Tiempo promedio: {stats['tiempo_promedio_allocacion']*1000:.2f}ms")
    print(f"Liberaciones: {stats['liberaciones_realizadas']}")
    print(f"  Tiempo promedio: {stats['tiempo_promedio_liberacion']*1000:.2f}ms")
    print(f"Búsquedas: {stats['busquedas_realizadas']}")
    print(f"  Tiempo promedio: {stats['tiempo_promedio_busqueda']*1000:.2f}ms")
    print()
    
    # Accesos al disco
    stats_disco = disco.obtener_estadisticas()
    print("=== ACCESOS AL DISCO ===")
    print(f"Lecturas: {stats_disco['accesos_lectura']}")
    print(f"Escrituras: {stats_disco['accesos_escritura']}")
    print(f"Total: {stats_disco['accesos_totales']}")
    print(f"\n¡Nota: MUCHAS MENOS lecturas que el Bitmap!")