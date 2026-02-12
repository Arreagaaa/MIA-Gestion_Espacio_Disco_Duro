class Nodo:
    def __init__(self, inicio, tamaño):
        self.inicio =inicio
        self.tamaño = tamaño
        self.siguiente = None


    def __str__(self):
        return f"Espacio[pos:{self.inicio}, tam:{self.tamaño}]"
    
    def __repr__(self):
        return self.__str__()
    


class NodoDoble:

    def __init__(self, inicio, tamaño):
        self.inicio = inicio
        self.tamaño = tamaño
        self.siguiente = None   # Apunta al siguiente espacio libre 
        self.anterior = None    # Apunta al espacio anterior

    def __str__(self):
        return f"Espacio[pos:{self.inicio}, tam:{self.tamaño}]"
    
    def __repr__(self):
        return self.__str__()    
    

def imprimir_lista_simple(cabeza):
    """
    Función de ayuda para visualizar una lista simple
    """
    if cabeza is None:
        print("Lista vacía (NULL)")
        return
        
    actual = cabeza
    elementos = []
    
    while actual is not None:
        elementos.append(str(actual))
        actual = actual.siguiente
    
    print(" -> ".join(elementos) + " -> NULL")


def imprimir_lista_doble(cabeza):
    """
    Función de ayuda para visualizar una lista doble
    """
    if cabeza is None:
        print("Lista vacia (NULL)")
        return
        
    actual = cabeza
    elementos = []
    
    while actual is not None:
        elementos.append(str(actual))
        actual = actual.siguiente
    
    print("NULL <- " + " ⇄ ".join(elementos) + " -> NULL")


