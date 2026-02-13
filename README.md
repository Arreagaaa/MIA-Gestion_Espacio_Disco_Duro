# Gestión de espacio en disco (Simulador)

Este repositorio contiene un proyecto educativo que compara tres métodos
para gestionar espacio libre en un disco simulado. Está pensado para
usuarios que quieren ejecutar la simulación y revisar resultados sin
entrar en detalles de implementación.

Contenido principal
- `SIMULACION_CPP/`: Implementación en C++ (compilable y ejecutable).
- `SIMULACION_PY/`: Implementación en Python (mismos conceptos, versión más didáctica).
- `docs/`: Material del curso: enunciado del proyecto, presentación y resultados.

Resumen del proyecto (qué hace)
--------------------------------
El simulador crea un "disco" de 1024 bloques y compara tres estructuras de
gestión del espacio libre:

- Mapa de Bits: un array booleano que marca bloques ocupados/libres.
- Lista Simplemente Ligada: guarda huecos libres como nodos {inicio,tamaño}.
- Lista Doblemente Ligada: similar a la lista simple pero con puntero al
	anterior (mejora ciertas operaciones).

La simulación realiza por estructura:
- 50 asignaciones (tamaños aleatorios 1–32 bloques),
- 30 liberaciones aleatorias (de las asignaciones previas),
- 1 búsqueda del hueco libre más grande,
todo esto repetido en 5 corridas. Se miden tiempos e indicadores como
fragmentación y se guardan en `SIMULACION_CPP/data/resultados.txt`.

Cómo se implementó (breve, para usuarios)
-----------------------------------------
C++ (carpeta `SIMULACION_CPP`)
- Código orientado a objetos con una clase base `GestorDisco` y tres
	implementaciones (`MapaDeBits`, `ListaSimple`, `ListaDoble`).
- Simula tiempos de I/O con pausas intencionales y mide tiempos con
	`chrono`. Guarda el estado inicial en `data/disco_inicial.txt` y los
	resultados en `data/resultados.txt`.

Python (carpeta `SIMULACION_PY`)
- Versión más pequeña y didáctica: misma idea (bitmap y listas), útil
	para explorar el comportamiento sin compilar.

Archivos importantes
-------------------
- `SIMULACION_CPP/simulador_disco` — ejecutable (si fue compilado).
- `SIMULACION_CPP/data/disco_inicial.txt` — estado inicial del disco.
- `SIMULACION_CPP/data/resultados.txt` — resultados y promedios.
- `docs/` — incluye el enunciado del proyecto, la presentación y los
	resultados adicionales.

Cómo ejecutar (C++)
-------------------
Abrir terminal y ejecutar:

```bash
cd SIMULACION_CPP
make        # compila
make run    # compila (si hace falta) y ejecuta la simulación
# o ejecutar el binario si ya fue compilado:
./simulador_disco
```

Ver resultados:

```bash
cat SIMULACION_CPP/data/resultados.txt
cat SIMULACION_CPP/data/disco_inicial.txt
```

Cómo ejecutar (Python)
---------------------
```bash
cd SIMULACION_PY
python3 main.py
```

Notas para la presentación (qué decir)
-------------------------------------
- Arquitectura: clase base y tres implementaciones — permite comparar
	fácilmente usando polimorfismo.
- Qué mide: tiempos de asignación, liberación y búsqueda, y fragmentación.
- Precisión experimental: el C++ asegura que todas las estructuras
	comienzan con el mismo estado inicial para una comparación justa.

Problemas conocidos y validaciones
----------------------------------
- El código fue revisado para asegurar que las liberaciones correspondan
	a las asignaciones (se guarda la posición real de la asignación).
- La función de fragmentación fue reforzada para evitar valores fuera de
	0–100%.

Si necesitas que haga una versión más corta para la diapositiva o que
genere gráficos a partir de `resultados.txt`, dímelo y lo preparo.

---
Archivo generado automáticamente por el equipo. Para cambios técnicos,
revisa `SIMULACION_CPP/src` y `SIMULACION_PY/`.
