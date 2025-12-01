# 🕹️ QuadTree (_Segundo Entregable_)
- Valeria Lock

Este proyecto muestra la estructura de datos **QuadTree** aplicada al entorno de videojuegos para optimizar consultas espaciales como **detección de colisiones** y **búsquedas de enemigos cercanos**, logrando mayor eficiencia que comparar todas las entidades entre sí (*O(n²)*).

---

## 🎯 ¿Qué problema resuelve?

En videojuegos con muchos objetos móviles (enemigos, balas, asteroides, pickups), la detección de interacción puede volverse lenta si se evalúan todas las combinaciones.  
El **QuadTree** permite dividir el espacio 2D en regiones y consultar solo el cuadrante relevante, reduciendo comparaciones y acelerando la lógica del juego.

---

## 🧠 ¿Cómo funciona a nivel general?

La pantalla mantiene un árbol de subdivisión recursiva:
- Si un nodo supera su capacidad, se subdivide en 4 hijos.
- Cada entidad se inserta según su posición 2D.
- Las consultas (`query(range)`) retornan solo enemigos cercanos a un área.

### Complejidad esperada:
|    Operación     | Complejidad promedio |           Peor caso         |
|------------------|----------------------|-----------------------------|
|     `insert`     |       *O(log n)*     | *O(n)* (desbalance extremo) |
| `search / query` |       *O(log n)*     |             *O(n)*          |
|   `subdivide`    |    *O(1)* por nodo   |   Depende de inserciones    |

---

## Modos

### **1 — Quadtree**
- Muestra entidades (**círculos simples**)
- Permite ver:
  - Límites de cuadrantes
  - Subdivisión recursiva
  - Zonas consultadas
- **Tecla [SPACE]**: alterna la visualización de la **cuadrilla** del QuadTree.
- **Tecla [M]**: Regresa al menú
- **Tecla [ESC]**: Cierra el programa

### **2 — Modo Arcade **
- Si el jugador toca cualquier enemigo, se produce **Game Over**.
- Al colisionar se fucionan y se crea un enemigo más grande.
- Tras unos segundos, aparecen más enemigos automáticamente para demostrar uso del QuadTree bajo carga.
- **Spawneo seguro**: no aparecen enemigos en el centro spawnea el jugador.
- **Fusions de enemigos cuando colisionan entre sí**:
  - Cambian de color según nivel de fusión:  
    🟩 Verde → 🟨 Amarillo → 🟧 Naranja → 🟥 Rojo → 🟪 Morado → 🟦 Azul
- **Timer**: muestra tiempo transcurrido de la partida.
- **Teclas [WASD | Flechas]**: controles de movimiento
- **Tecla [SPACE]**: alterna la visualización de la **cuadrilla** del QuadTree.
- **Tecla [R]**: Reiniciar la partida (resetea el jugador, enemigos y tiempo)
- **Tecla [M]**: Regresa al menú
- **Tecla [ESC]**: Cierra el programa

---

## ⚙️ Guía de ejecución

### Requisitos
- Compilador: C++17 (`g++ 17` o `MSVC 2022`)
- Librería:  
  - `SFML 3.x` (instalada por vcpkg o MSYS2 UCRT64)
- Archivos obligatorios:  
├── main.cpp
├── QuadTree.h
├── Entity.h
├── fondo.jpg         # Imagen de fondo
├── PIXEL.ttf         # Fuente principal
└── ARIAL.TTF         # Fuente alternativa

### Compilación

Desde la terminal **MSYS2 UCRT64**:

cd /ruta/al/proyecto

g++ -std=c++17 main.cpp -o quadtree_game_menu \
    -lsfml-graphics -lsfml-window -lsfml-system

SFML debe estar instalada en el entorno UCRT64 y que las DLL necesarias (por ejemplo, `sfml-graphics-3.dll`, `sfml-window-3.dll`, `sfml-system-3.dll`, `libgcc_s_seh-1.dll`, etc.) estén en el `PATH` o en la misma carpeta que `quadtree_game_menu.exe`.
Luego ejecutar:

./quadtree_game_menu

---

## Casos de prueba y resultados

### 1. Inserción y subdivisión en modo Debug

- **Acción:** Ejecutar el programa, elegir opción `1` (Debug) y observar la pantalla.
- **Esperado:**
  - Todas las bolitas se ven dentro del área de la ventana.
  - La cuadrícula se mantiene oculta hasta que se presiona `ESPACIO`.
  - Al presionar `ESPACIO`, aparecen las líneas del QuadTree, subdividiendo el espacio en cuadrantes según la cantidad de objetos.
- **Resultado observado:**  
  - El QuadTree se subdivide dinámicamente donde hay más bolitas, mostrando un uso eficiente del espacio.

### 2. Detección de colisiones optimizada

- **Acción:** En modo Debug (opción `1`), dejar que las bolitas se muevan hasta que varias se acerquen.
- **Esperado:**
  - Cuando dos bolitas se acercan a menos de **2×radio**, ambas cambian su color a **rojo**.
  - La detección se realiza usando **queries sobre el QuadTree**, no comparando todas contra todas.
- **Resultado observado:**  
  - Las colisiones se marcan en rojo localmente y no se detectan colisiones falsas en zonas alejadas.

### 3. Restricción de spawn en modo Arcade

- **Acción:** Elegir opción `2` (Arcade) varias veces y observar dónde aparecen los enemigos.
- **Esperado:**
  - El jugador aparece en el **centro** de la pantalla.
  - Ningún enemigo aparece dentro de un **radio de seguridad** alrededor del jugador.
- **Resultado observado:**  
  - Siempre hay una zona despejada alrededor del jugador al inicio, mejorando la jugabilidad.

### 4. Fusión y cambio de color de marcianitos

- **Acción:** En modo Arcade, dejar que los marcianitos se muevan y choquen entre sí.
- **Esperado:**
  - Cuando dos enemigos se solapan, se genera un **marciano más grande**.
  - El color evoluciona en la secuencia:
    `Verde → Amarillo → Naranja → Rojo → Morado → Azul`.
  - El número total de marcianitos disminuye al producirse fusiones.
- **Resultado observado:**  
  - Se observan enemigos cada vez más grandes y de otros colores, lo que evidencia el proceso de fusión.

### 5. Game Over y reinicio

- **Acción:** Colisionar con un enemigo en modo Arcade.
- **Esperado:**
  - El jugador cambia de estado (Game Over) y se muestra un mensaje de **GAME OVER** con el tiempo alcanzado.
  - Al presionar `R`, la partida se reinicia y el tiempo vuelve a `00:00.00`.
- **Resultado observado:**  
  - El flujo Game Over → Reinicio funciona correctamente y el QuadTree se reconstruye con las nuevas entidades.

---

## Fotos

1. **Menú principal**  
   ![Menú principal](fotos/menu.jpg)
2. **Modo Debug – sin cuadrícula**  
   ![Modo Debug sin cuadrícula](fotos/debug_sin.jpg)
3. **Modo Debug – con cuadrícula**  
   ![Modo Debug con cuadrícula](fotos/debug_con.jpg)
4. **Modo Arcade – sin cuadrícula**  
   ![Modo Arcade sin cuadrícula](fotos/arcade_sin.jpg)
5. **Modo Arcade – con cuadrícula**  
   ![Modo Arcade con cuadrícula](fotos/arcade_con.jpg)

--- 

## Resumen

El QuadTree se utiliza tanto en el modo Debug como en el modo Arcade para:

- Insertar entidades dinámicas.
- Hacer consultas por región alrededor de cada entidad.
- Reducir el costo de detección de colisiones y fusiones frente a un enfoque ingenuo `O(n²)` sin estructura.

Este proyecto demuestra la aplicación práctica de una estructura de datos avanzada dentro de un entorno lúdico, facilitando la comprensión visual de su funcionamiento.
