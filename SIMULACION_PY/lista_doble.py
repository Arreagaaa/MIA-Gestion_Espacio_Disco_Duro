"""
LISTA DOBLEMENTE LIGADA - Estructura de gestión de espacio

Similar a la Lista Simple, pero cada nodo tiene punteros en
ambas direcciones (anterior ← y siguiente →), lo que facilita
operaciones de inserción y fusión.

VENTAJAS sobre Lista Simple:
- Navegación bidireccional (← →)
- Inserción/fusión más eficiente
- Mejor para liberar bloques adyacentes

DESVENTAJAS:
- Usa más memoria (2 punteros por nodo vs 1)
- Ligeramente más compleja de mantener
"""

import time
from disco_base import DiscoSimulado
from nodo import NodoDoble, imprimir_lista_doble


class ListaDoble:
    """
    Gestión de espacio libre usando Lista Doblemente Ligada
    
    Mantiene una lista donde cada nodo representa un hueco libre
    y puede navegar en ambas direcciones.
    
    Atributos:
        disco (DiscoSimulado): Referencia al disco simulado
        cabeza (NodoDoble): Primer nodo de la lista
        cola (NodoDoble): Último nodo de la lista (útil para algunas operaciones)
        nombre (str): Nombre de la estructura
    """
    
    def __init__(self, disco):
        """
        Inicializa la lista doblemente ligada con un disco
        
        Args:
            disco (DiscoSimulado): El disco a gestionar
        """
        self.disco = disco
        self.cabeza = None
        self.cola = None  # Útil para agregar al final rápidamente
        self.nombre = "Lista Doble"
        
        # Estadísticas
        self.tiempo_allocacion = []
        self.tiempo_liberacion = []
        self.tiempo_busqueda = []
        
        # Construir lista inicial
        self._construir_lista_inicial()
    
    
    def _construir_lista_inicial(self):
        """
        Escanea el disco y construye la lista de huecos libres
        
        Similar a Lista Simple, pero mantiene enlaces bidireccionales
        """
        hueco_inicio = -1
        hueco_tamanio = 0
        
        for pos in range(self.disco.TAMANIO):
            esta_ocupado = self.disco.leer_directo(pos)
            
            if not esta_ocupado:  # Bloque LIBRE
                if hueco_tamanio == 0:
                    hueco_inicio = pos
                hueco_tamanio += 1
                
            else:  # Bloque OCUPADO
                if hueco_tamanio > 0:
                    self._insertar_nodo_ordenado(hueco_inicio, hueco_tamanio)
                    hueco_tamanio = 0
        
        # Último hueco
        if hueco_tamanio > 0:
            self._insertar_nodo_ordenado(hueco_inicio, hueco_tamanio)
    
    
    def _insertar_nodo_ordenado(self, inicio, tamanio):
        """
        Inserta un nuevo nodo manteniendo el orden y enlaces bidireccionales
        
        La ventaja de la lista doble es que podemos actualizar
        los enlaces anterior/siguiente más fácilmente.
        
        Args:
            inicio (int): Posición donde empieza el hueco
            tamanio (int): Cantidad de bloques del hueco
        """
        nuevo_nodo = NodoDoble(inicio, tamanio)
        
        # CASO 1: Lista vacía
        if self.cabeza is None:
            self.cabeza = nuevo_nodo
            self.cola = nuevo_nodo
            return
        
        # CASO 2: Insertar al inicio (antes de cabeza)
        if inicio < self.cabeza.inicio:
            nuevo_nodo.siguiente = self.cabeza
            self.cabeza.anterior = nuevo_nodo
            self.cabeza = nuevo_nodo
            return
        
        # CASO 3: Insertar al final (después de cola)
        if inicio > self.cola.inicio:
            self.cola.siguiente = nuevo_nodo
            nuevo_nodo.anterior = self.cola
            self.cola = nuevo_nodo
            return
        
        # CASO 4: Insertar en medio
        actual = self.cabeza
        while actual.siguiente is not None and actual.siguiente.inicio < inicio:
            actual = actual.siguiente
        
        # Insertar después de 'actual'
        nuevo_nodo.siguiente = actual.siguiente
        nuevo_nodo.anterior = actual
        
        if actual.siguiente is not None:
            actual.siguiente.anterior = nuevo_nodo
        
        actual.siguiente = nuevo_nodo
    
    
    # ========================================
    # OPERACIÓN 1: ALLOCAR BLOQUES
    # ========================================
    
    def allocar(self, n_bloques):
        """
        Encuentra y reserva n bloques consecutivos libres
        
        ALGORITMO (igual que Lista Simple):
        1. Recorrer la lista de nodos
        2. Buscar el primer nodo con tamanio >= n_bloques
        3. Usar ese nodo (total o parcialmente)
        4. Actualizar enlaces bidireccionales
        
        Args:
            n_bloques (int): Cantidad de bloques necesarios
            
        Returns:
            tuple: (inicio, tamanio) si se allocó
            None: Si no hay espacio suficiente
        """
        tiempo_inicio = time.time()
        
        if self.cabeza is None:
            tiempo_fin = time.time()
            self.tiempo_allocacion.append(tiempo_fin - tiempo_inicio)
            return None
        
        # Buscar nodo adecuado
        actual = self.cabeza
        
        while actual is not None:
            
            if actual.tamaño >= n_bloques:
                
                inicio_allocacion = actual.inicio
                
                # CASO A: Usar TODO el nodo
                if actual.tamaño == n_bloques:
                    # Eliminar nodo de la lista
                    self._eliminar_nodo(actual)
                
                # CASO B: Usar PARTE del nodo
                else:
                    actual.inicio += n_bloques
                    actual.tamaño -= n_bloques
                
                # Marcar bloques como ocupados en el disco
                for i in range(n_bloques):
                    self.disco.escribir_bloque(inicio_allocacion + i, True)
                
                tiempo_fin = time.time()
                self.tiempo_allocacion.append(tiempo_fin - tiempo_inicio)
                return (inicio_allocacion, n_bloques)
            
            actual = actual.siguiente
        
        # No se encontró espacio
        tiempo_fin = time.time()
        self.tiempo_allocacion.append(tiempo_fin - tiempo_inicio)
        return None
    
    
    def _eliminar_nodo(self, nodo):
        """
        Elimina un nodo de la lista actualizando enlaces bidireccionales
        
        VENTAJA DE LISTA DOBLE:
        Podemos eliminar un nodo conociendo solo el nodo mismo,
        sin necesitar referencia al nodo anterior.
        
        Args:
            nodo (NodoDoble): El nodo a eliminar
        """
        # Actualizar nodo anterior
        if nodo.anterior is not None:
            nodo.anterior.siguiente = nodo.siguiente
        else:
            # Era la cabeza
            self.cabeza = nodo.siguiente
        
        # Actualizar nodo siguiente
        if nodo.siguiente is not None:
            nodo.siguiente.anterior = nodo.anterior
        else:
            # Era la cola
            self.cola = nodo.anterior
    
    
    # ========================================
    # OPERACIÓN 2: LIBERAR BLOQUES
    # ========================================
    
    def liberar(self, inicio, tamaño):
        """
        Marca bloques como libres y los agrega a la lista
        
        VENTAJA DE LISTA DOBLE:
        La fusión con nodos adyacentes es más eficiente porque
        podemos navegar hacia atrás sin perder referencias.
        
        Args:
            inicio (int): Posición del primer bloque a liberar
            tamanio (int): Cantidad de bloques
            
        Returns:
            bool: True si se liberó exitosamente
        """
        tiempo_inicio = time.time()
        
        # Validar
        if inicio < 0 or inicio >= self.disco.TAMANIO:
            return False
        if inicio + tamaño > self.disco.TAMANIO:
            return False
        
        # Marcar bloques como libres
        for i in range(tamaño):
            self.disco.escribir_bloque(inicio + i, False)
        
        # Agregar/fusionar en la lista
        self._fusionar_hueco(inicio, tamaño)
        
        tiempo_fin = time.time()
        self.tiempo_liberacion.append(tiempo_fin - tiempo_inicio)
        return True
    
    
    def _fusionar_hueco(self, inicio, tamaño):
        """
        Inserta un hueco libre fusionándolo con nodos adyacentes
        
        VENTAJA DE LISTA DOBLE:
        Podemos navegar hacia atrás fácilmente para fusionar
        con nodos anteriores.
        
        Casos de fusión:
        1. Solo con anterior
        2. Solo con siguiente
        3. Con ambos (anterior y siguiente)
        4. Sin fusión (insertar como nuevo nodo)
        """
        fin = inicio + tamaño - 1
        
        # CASO ESPECIAL: Lista vacía
        if self.cabeza is None:
            self._insertar_nodo_ordenado(inicio, tamaño)
            return
        
        # Buscar posición adecuada
        actual = self.cabeza
        nodo_anterior = None
        nodo_siguiente = None
        
        # Encontrar nodos adyacentes
        while actual is not None:
            
            # ¿Este nodo está justo antes del hueco?
            if actual.inicio + actual.tamaño == inicio:
                nodo_anterior = actual
            
            # ¿Este nodo está justo después del hueco?
            if fin + 1 == actual.inicio:
                nodo_siguiente = actual
            
            actual = actual.siguiente
        
        
        # FUSIÓN CON AMBOS (anterior y siguiente)
        if nodo_anterior is not None and nodo_siguiente is not None:
            # Expandir nodo_anterior para incluir hueco y nodo_siguiente
            nodo_anterior.tamaño += tamaño + nodo_siguiente.tamaño
            # Eliminar nodo_siguiente (ya fusionado)
            self._eliminar_nodo(nodo_siguiente)
            return
        
        # FUSIÓN SOLO CON ANTERIOR
        if nodo_anterior is not None:
            nodo_anterior.tamaño += tamaño
            return
        
        # FUSIÓN SOLO CON SIGUIENTE
        if nodo_siguiente is not None:
            nodo_siguiente.inicio = inicio
            nodo_siguiente.tamaño += tamaño
            return
        
        # SIN FUSIÓN: Insertar como nuevo nodo
        self._insertar_nodo_ordenado(inicio, tamaño)
    
    
    # ========================================
    # OPERACIÓN 3: BUSCAR HUECO MÁS GRANDE
    # ========================================
    
    def buscar_mayor_hueco(self):
        """
        Encuentra el hueco libre más grande
        
        ALGORITMO (igual que Lista Simple):
        Recorrer la lista y trackear el nodo con mayor tamanio
        
        Returns:
            tuple: (inicio, tamanio) del hueco más grande
            None: Si no hay bloques libres
        """
        tiempo_inicio = time.time()
        
        if self.cabeza is None:
            tiempo_fin = time.time()
            self.tiempo_busqueda.append(tiempo_fin - tiempo_inicio)
            return None
        
        mayor_nodo = self.cabeza
        actual = self.cabeza.siguiente
        
        while actual is not None:
            if actual.tamaño > mayor_nodo.tamaño:
                mayor_nodo = actual
            actual = actual.siguiente
        
        tiempo_fin = time.time()
        self.tiempo_busqueda.append(tiempo_fin - tiempo_inicio)
        
        return (mayor_nodo.inicio, mayor_nodo.tamaño)
    
    
    # ========================================
    # UTILIDADES
    # ========================================
    
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
        """Muestra la lista de huecos libres"""
        if self.cabeza is None:
            print("Lista vacía (sin huecos libres)")
            return
        
        print("Lista de huecos libres:")
        actual = self.cabeza
        nodos = []
        while actual is not None:
            nodos.append(f"[{actual.inicio}:{actual.inicio+actual.tamaño-1}]({actual.tamaño})")
            actual = actual.siguiente
        print("NULL ← " + " ⇄ ".join(nodos) + " → NULL")
    
    
    def verificar_integridad(self):
        """
        Verifica que los enlaces bidireccionales sean consistentes
        (útil para debugging)
        """
        if self.cabeza is None:
            return True
        
        # Verificar hacia adelante
        actual = self.cabeza
        while actual.siguiente is not None:
            if actual.siguiente.anterior != actual:
                print(f"ERROR: Enlace inconsistente en nodo {actual}")
                return False
            actual = actual.siguiente
        
        # Verificar que llegamos a la cola
        if actual != self.cola:
            print(f"ERROR: Cola incorrecta")
            return False
        
        return True
    
    
    def __str__(self):
        """Representación en string"""
        return f"Lista Doble ({self.contar_nodos()} huecos libres)"


# ============================================
# PRUEBAS DE LISTA DOBLE
# ============================================

if __name__ == "__main__":
    print("=== PRUEBA DE LISTA DOBLE ===\n")
    
    # Crear disco
    disco = DiscoSimulado()
    disco.inicializar_aleatorio(0.50)
    
    # Crear lista doble
    print("Construyendo lista de huecos libres...")
    lista = ListaDoble(disco)
    print(f"✓ {lista}\n")
    
    # Verificar integridad
    if lista.verificar_integridad():
        print("✓ Enlaces bidireccionales verificados\n")
    else:
        print("✗ ERROR en enlaces\n")
    
    # Visualizar lista inicial (primeros nodos)
    print("Primeros nodos de la lista:")
    actual = lista.cabeza
    count = 0
    while actual is not None and count < 10:
        prev_str = f"{actual.anterior.inicio}" if actual.anterior else "NULL"
        next_str = f"{actual.siguiente.inicio}" if actual.siguiente else "NULL"
        print(f"  {prev_str} ← [{actual.inicio}:{actual.inicio+actual.tamaño-1}]({actual.tamaño}) → {next_str}")
        actual = actual.siguiente
        count += 1
    print("  ...\n")
    
    # Estado del disco
    stats_disco = disco.obtener_estadisticas()
    print(f"Estado del disco:")
    print(f"  Ocupados: {stats_disco['bloques_ocupados']}")
    print(f"  Libres: {stats_disco['bloques_libres']}\n")
    
    # PRUEBA 1: Buscar hueco más grande
    print("--- PRUEBA 1: Buscar hueco más grande ---")
    hueco = lista.buscar_mayor_hueco()
    if hueco:
        print(f"✓ Hueco más grande: {hueco[1]} bloques en posición {hueco[0]}\n")
    
    # PRUEBA 2: Allocar bloques
    print("--- PRUEBA 2: Allocar bloques ---")
    allocaciones = []
    tamanios = [8, 5, 12]
    
    for tam in tamanios:
        print(f"Allocando {tam} bloques...", end=" ")
        resultado = lista.allocar(tam)
        if resultado:
            allocaciones.append(resultado)
            print(f"✓ Posición {resultado[0]}")
        else:
            print("✗ Sin espacio")
    
    print(f"\n✓ Lista ahora tiene {lista.contar_nodos()} nodos\n")
    
    # PRUEBA 3: Liberar bloques con fusión
    if len(allocaciones) >= 2:
        print("--- PRUEBA 3: Liberar bloques (con fusión) ---")
        
        print(f"Nodos antes de liberar: {lista.contar_nodos()}")
        
        # Liberar el primero
        inicio1, tam1 = allocaciones[0]
        print(f"Liberando {tam1} bloques desde posición {inicio1}...")
        lista.liberar(inicio1, tam1)
        
        # Liberar el segundo
        inicio2, tam2 = allocaciones[1]
        print(f"Liberando {tam2} bloques desde posición {inicio2}...")
        lista.liberar(inicio2, tam2)
        
        print(f"Nodos después de liberar: {lista.contar_nodos()}")
        print("(Si se fusionaron con nodos adyacentes, habrá menos nodos)\n")
    
    # Verificar integridad después de operaciones
    if lista.verificar_integridad():
        print("✓ Integridad de enlaces verificada\n")
    
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
    print(f"\n¡Rendimiento similar a Lista Simple!")