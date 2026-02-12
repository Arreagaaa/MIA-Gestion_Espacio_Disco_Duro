"""
PROGRAMA PRINCIPAL
Simulador de Gestión de Espacio en Disco Duro

Proyecto para: Manejo e Implementación de Archivos
Fecha: 12 de Febrero 2026
"""

import sys
import os


def mostrar_menu():
    """Muestra el menú principal"""
    print("\n" + "="*60)
    print("SIMULADOR DE GESTIÓN DE ESPACIO EN DISCO")
    print("="*60)
    print()
    print("1. Ejecutar simulador completo (5 corridas)")
    print("2. Ejecutar simulador rápido (recomendado)")
    print("3. Probar Bitmap individualmente")
    print("4. Probar Lista Simple individualmente")
    print("5. Probar Lista Doble individualmente")
    print("6. Ver resultados anteriores")
    print("7. Salir")
    print()
    print("="*60)


def ejecutar_opcion(opcion):
    """Ejecuta la opción seleccionada"""
    
    if opcion == '1':
        print("\nEl simulador completo puede tardar varios minutos")
        print("debido a que el Bitmap es muy lento.")
        respuesta = input("¿Continuar? (s/n): ")
        if respuesta.lower() == 's':
            print("\nEjecutando simulador completo...")
            os.system("python simulador.py")
        
    elif opcion == '2':
        print("\nProbando Bitmap...")
        os.system("python bitmap.py")
        
    elif opcion == '3':
        print("\nProbando Lista Simple...")
        os.system("python lista_simple.py")
        
    elif opcion == '4':
        print("\nProbando Lista Doble...")
        os.system("python lista_doble.py")
        
    elif opcion == '5':
        print("\n" + "="*60)
        print("RESULTADOS ANTERIORES")
        print("="*60)
        try:
            with open("datos/resultados.txt", 'r', encoding='utf-8') as f:
                print(f.read())
        except FileNotFoundError:
            print("\n  No hay resultados guardados.")
            print("Ejecuta primero el simulador (opción 1 o 2)")
        
    elif opcion == '6':
        print("\n")
        sys.exit(0)
        
    else:
        print("\n Opción inválida")


def main():
    """Función principal"""
    print("\n")
    print("┌" + "─"*58 + "┐")
    print("│" + " "*10 + "GESTIÓN DE ESPACIO EN DISCO DURO" + " "*16 + "│")
    print("│" + " "*58 + "│")
    print("│" + " "*10 + "Proyecto: Manejo e Implementación de Archivos" + " "*3 + "│")
    print("└" + "─"*58 + "┘")
    
    while True:
        mostrar_menu()
        opcion = input("Selecciona una opción (1-6): ")
        ejecutar_opcion(opcion)
        
        if opcion in ['1', '2', '3', '4', '5', '6']:
            input("\nPresiona ENTER para continuar...")


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n\nPrograma interrumpido por el usuario")
        sys.exit(0)