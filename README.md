# SistOpe – Sistema Operativo Modular (Entrega 3)

**Asignatura:** Sistemas Operativos (INFO198)
**Profesor:** Dr. Luis Veas-Castillo
**Entrega:** 3 – Concurrencia, sincronización, procesamiento paralelo y sockets

---

## 1. Propósito de la aplicación

**SistOpe** es un sistema modular en C++17 desarrollado durante el semestre. Mantiene todas las funcionalidades de las Entregas 1 y 2 (usuarios, perfiles, autenticación por línea de comandos, menú principal, matrices, palíndromos, f(x), conteo de texto) y en esta **Entrega 3** incorpora:

| Problema | Funcionalidad | Dónde está |
| :--- | :--- | :--- |
| **1** | **Creación paralela del índice invertido** usando una cola de trabajo compartida, `N_THREADS` threads trabajadores y semáforos. | Menú principal → **opción 8 "Crear indice invertido"** · `src/indice_invertido.cpp`, `include/semaforo.h` |
| **2** | **Juego multiplayer "Carrera de Equipos"** con arquitectura cliente‑servidor mediante **sockets TCP**, equipos, turnos y archivo de log. | Menú principal → **opción 3 "Juego"** · `juego_server.exe`, `juego_cliente.exe` · `src/juego_*.cpp`, `src/red.cpp` |

### Cómo se integran las entregas: cada entrega llama a la anterior

SistOpe se construye de forma incremental: **cada entrega reutiliza lo ya construido ejecutándolo como un programa independiente mediante una llamada a sistema** (`system()`), en lugar de copiar su código. Esto es lo que pide la pauta de la Entrega 2 para la opción 1 (*"debe realizar una llamada a sistema 'ADMINISTRACIÓN DE USUARIOS Y PERFILES', que ya está construido en la entrega 1"*) y para el multiplicador (*"debe ser otro programa, que sea llamado desde el menú"*), y se mantiene en la Entrega 3:

```
 Entrega 1                Entrega 2                            Entrega 3
 ─────────                ─────────                            ─────────
                          sistope.exe  (MENÚ PRINCIPAL: login -u -p -f, perfiles, opciones 0-8)
 admin_usuarios.exe  ◄──── (1) llamada a sistema  ADMIN_EXEC        [usuario, perfil]
                          multi.exe ◄─ (2) llamada a sistema  MATRIX_EXEC       [A, B, sep, usuario, perfil]
                                       (3) llamada a sistema  GAME_CLIENT_EXEC ──► juego_cliente.exe [usuario]
                                           llamada a sistema  GAME_SERVER_EXEC ──► juego_server.exe
                                       (8) llamada a sistema  INDEX_EXEC       ──► indice_invertido.exe [usuario, perfil]
```

- La Entrega 2 (menú principal) **llama** al programa de la Entrega 1 (`admin_usuarios.exe`), que es el mismo sistema de administración construido en la Entrega 1 y que también puede ejecutarse solo.
- La Entrega 3 se incorpora **al sistema de la Entrega 2**: sus funcionalidades son programas propios (`indice_invertido.exe`, `juego_server.exe`, `juego_cliente.exe`) que el menú principal ejecuta, y el menú sigue llamando a los programas de las Entregas 1 y 2.
- Cada programa recibe por argumentos el **usuario y perfil** de la sesión y los muestra en pantalla. `admin_usuarios.exe` además rechaza (código 2) un perfil distinto de `ADMIN`, protegiendo la integridad del sistema aunque se ejecute directamente.
- El nombre de cada ejecutable se obtiene de una variable de entorno. La función común está en `src/externo.cpp` (arma el comando con comillas correctas para `cmd` de Windows y `sh` de Linux, informa si el programa no existe y devuelve su código de salida).
- Al volver de `admin_usuarios.exe`, el menú vuelve a leer `USUARIOS.TXT` y `PERFILES.TXT`, porque el programa externo pudo modificarlos.

Todas las configuraciones nuevas se leen desde **variables de entorno** (archivo `.env` o variables del sistema operativo); ningún nombre de archivo, cantidad de threads ni parámetro del juego está escrito en el código.

### Menú principal

```
0) Salir
1) Admin de usuarios y perfiles      (llama a admin_usuarios.exe) [Entrega 1, solo ADMIN]
2) Multiplica matrices NxM            (llama a multi.exe)          [Entrega 2]
3) Juego                              (llama a juego_cliente.exe / juego_server.exe) [Entrega 3 - Problema 2]
4) Es palindromo?                                                  [Entrega 2]
5) Calcular f(x)=x*x + 2x + 8                                      [Entrega 2]
6) CONTEO SOBRE TEXTO                 (archivo de la bandera -f)   [Entrega 2]
7) CONTEO SOBRE ARCHIVO                                            [Entrega 2]
8) Crear indice invertido             (llama a indice_invertido.exe) [Entrega 3 - Problema 1]
```

Los perfiles `ADMIN`, `GENERAL` y `DOCTOR` incluidos en `PERFILES.TXT` tienen habilitada la nueva opción 8.

---

## 2. Estructura del proyecto

Los archivos marcados con ★ son nuevos en la Entrega 3; los marcados con ✎ fueron modificados.

```text
SistOpe/
  Makefile ✎                  # compila los 6 programas
  README.md ✎
  .env ✎                      # variables de entorno (Entregas 1, 2 y 3)
  USUARIOS.TXT / PERFILES.TXT ✎ # datos binarios (se agregó la opción 8 a los perfiles)
  a.txt / b.txt               # matrices de prueba
  LIBROS/                     # 97 libros .txt (> 50 MB) usados por el índice invertido

  include/
    semaforo.h ★              # semáforo portable (POSIX sem_t / Win32 / macOS)
    indice_invertido.h ★      # Problema 1
    red.h ★                   # capa de sockets portable (POSIX / Winsock2)
    juego.h ★                 # Problema 2: configuración, cliente, menú del juego
    externo.h ★               # llamada a sistema a los programas de cada entrega
    admin_usuarios.h ★        # Entrega 1 como programa independiente
    cli.h conteo.h env.h matriz.h menu.h perfil.h usuario.h utilidades.h campos.h

  src/                        (programa al que pertenece cada archivo)
    admin_main.cpp ★          # Entrega 1   -> main de admin_usuarios.exe
    admin_usuarios.cpp ★      # Entrega 1   -> pantallas de gestión de usuarios y perfiles (antes en menu.cpp)
    main.cpp ✎ menu.cpp ✎     # Entrega 2   -> sistope.exe (MENÚ PRINCIPAL)
    externo.cpp ★             # Entrega 2/3 -> llamada a sistema a los demás programas
    multi_main.cpp matriz.cpp # Entrega 2   -> multi.exe
    indice_main.cpp ★         # Entrega 3.1 -> main de indice_invertido.exe
    indice_invertido.cpp ★    # Entrega 3.1 -> cola, threads trabajadores, semáforos, índice
    juego_servidor.cpp ★      # Entrega 3.2 -> juego_server.exe
    juego_cliente_main.cpp ★  # Entrega 3.2 -> main de juego_cliente.exe
    juego_cliente.cpp ★       # Entrega 3.2 -> cliente (2 threads: receptor y teclado)
    juego_menu.cpp ★          # Entrega 3.2 -> submenú "3) Juego" de sistope.exe
    juego_config.cpp ★        # Entrega 3.2 -> lectura/validación de variables del juego
    red.cpp ★                 # Entrega 3.2 -> sockets TCP
    conteo.cpp ✎  env.cpp ✎
    cli.cpp perfil.cpp usuario.cpp utilidades.cpp

  tools/
    migrar_txt_a_dat.py ✎     # genera PERFILES.DAT / USUARIOS.DAT (structs de 116 y 144 bytes)

  Archivos generados al ejecutar (no se versionan):
    indice_invertido.idx      # índice invertido   (INVERTED_INDEX_FILE)
    libros_mapeo.txt          # mapeo ID;libro     (BOOK_MAP_FILE)
    juego.log                 # log del servidor   (GAME_LOG_FILE)
```

---

## 3. Variables de entorno

El programa lee primero la variable desde el **sistema operativo**; si no existe allí la toma del archivo **`.env`** de la carpeta donde se ejecuta. Antes de usar cada valor se valida que **exista** y que sea **válido**; si no, se muestra un mensaje claro y la operación no comienza.

### 3.1 Entregas 1 y 2

| Variable | Descripción | Valor en `.env` |
| :--- | :--- | :--- |
| `USER_FILE` | Archivo binario de usuarios | `USUARIOS.TXT` |
| `PERFIL_FILE` | Archivo binario de perfiles | `PERFILES.TXT` |
| `DEFAULT_TEXT_FILE` | Archivo para la opción 6 si no se usa `-f` | `LIBROS/drama_don_quijote_de_la_mancha_cervantes.txt` |
| `ADMIN_EXEC` | Programa de la Entrega 1 que ejecuta la opción 1 | `admin_usuarios.exe` |
| `MATRIX_EXEC` | Ejecutable del multiplicador de matrices (opción 2) | `multi.exe` |

### 3.2 Entrega 3 – Problema 1 (índice invertido)

| Variable | Descripción | Validación | Valor en `.env` |
| :--- | :--- | :--- | :--- |
| `INDEX_EXEC` | Programa que ejecuta la opción 8 | obligatoria para la opción 8 | `indice_invertido.exe` |
| `INVERTED_INDEX_FILE` | Nombre/path del archivo de salida del índice invertido | obligatoria, no puede ser una carpeta, debe poder escribirse | `indice_invertido.idx` |
| `BOOK_MAP_FILE` | Nombre/path del archivo con la asociación `ID;nombre_libro` | obligatoria, distinta de `INVERTED_INDEX_FILE` | `libros_mapeo.txt` |
| `BOOKS_DIR` | Carpeta con los libros a procesar | obligatoria, debe existir, ser carpeta, poder leerse y tener `.txt` | `LIBROS` |
| `N_THREADS` | Cantidad de threads trabajadores | obligatoria, entero entre 1 y 1024 | `4` |

### 3.3 Entrega 3 – Problema 2 (juego)

| Variable | Descripción | Validación | Valor en `.env` |
| :--- | :--- | :--- | :--- |
| `GAME_SERVER_IP` | IP o nombre del servidor al que se conectan los clientes | obligatoria | `127.0.0.1` |
| `GAME_PORT` | Puerto TCP del servidor | 1 – 65535 | `8080` |
| `GAME_LOG_FILE` | Nombre/path del archivo de log del juego | obligatoria, debe poder abrirse | `juego.log` |
| `GAME_SERVER_EXEC` | Ejecutable del servidor (lo inicia la opción 3 → 2) | obligatoria para esa opción | `juego_server.exe` |
| `GAME_CLIENT_EXEC` | Ejecutable del cliente (lo ejecuta la opción 3 → 1) | obligatoria para esa opción | `juego_cliente.exe` |
| `GAME_BOARD_SIZE` | Tamaño del tablero **X** (meta) | 5 – 1000 | `30` |
| `GAME_DICE_FACES` | Caras **R** del dado | 2 – 100 | `6` |
| `GAME_MIN_PLAYERS` | Cantidad mínima de jugadores | 2 – 100, ≥ `MIN_TEAMS × MIN_PLAYERS_PER_TEAM` | `2` |
| `GAME_MAX_PLAYERS` | Cantidad máxima de jugadores | ≥ mínimo, ≤ `MAX_TEAMS × MAX_PLAYERS_PER_TEAM` | `8` |
| `GAME_MIN_TEAMS` | Cantidad mínima de equipos con jugadores | 2 – 26, ≤ máximo | `2` |
| `GAME_MAX_TEAMS` | Cantidad máxima de equipos (EquipoA, EquipoB, …) | 2 – 26 | `4` |
| `GAME_MIN_PLAYERS_PER_TEAM` | Mínimo de jugadores por equipo | 1 – 50, ≤ máximo | `1` |
| `GAME_MAX_PLAYERS_PER_TEAM` | Máximo de jugadores por equipo | 1 – 50 | `2` |
| `GAME_TURN_TIMEOUT` | Segundos para lanzar en cada turno (`0` = sin límite) | 0 – 3600 | `60` |

Ejemplo para sobrescribir una variable sin editar `.env`:

```bash
N_THREADS=8 ./sistope.exe -u lve -p 123456          # Linux / WSL
$env:N_THREADS=8; .\sistope.exe -u lve -p 123456    # Windows PowerShell
```

---

## 4. Compilación

Requisitos: `g++` con soporte C++17 (GCC 9 o superior). En Windows se recomienda **MSYS2 / MinGW‑w64** (modelo de threads *posix*, que es el de MSYS2).

> El proyecto se entrega con los 6 programas **ya compilados para Windows** (MinGW‑w64, enlazados estáticamente, no requieren DLL). En Linux/WSL basta ejecutar `make`, que los vuelve a generar como ejecutables de Linux.

### Linux / WSL / macOS

```bash
make            # genera admin_usuarios.exe, sistope.exe, multi.exe, indice_invertido.exe, juego_server.exe y juego_cliente.exe
make clean      # elimina los ejecutables
```

### Windows (PowerShell o CMD con MinGW‑w64)

Con `mingw32-make` basta ejecutar `mingw32-make` (el Makefile agrega `-lws2_32` automáticamente). Compilación directa:

```powershell
g++ -std=c++17 -O2 -Wall -static -Iinclude src/admin_main.cpp src/admin_usuarios.cpp src/usuario.cpp src/perfil.cpp src/env.cpp -o admin_usuarios.exe

g++ -std=c++17 -O2 -Wall -static -Iinclude src/main.cpp src/usuario.cpp src/perfil.cpp src/menu.cpp src/env.cpp src/cli.cpp src/conteo.cpp src/utilidades.cpp src/externo.cpp src/juego_menu.cpp src/juego_config.cpp -o sistope.exe

g++ -std=c++17 -O2 -Wall -static -Iinclude src/multi_main.cpp src/matriz.cpp -o multi.exe

g++ -std=c++17 -O2 -Wall -static -Iinclude src/indice_main.cpp src/indice_invertido.cpp src/env.cpp -o indice_invertido.exe

g++ -std=c++17 -O2 -Wall -static -Iinclude src/juego_servidor.cpp src/juego_config.cpp src/red.cpp src/env.cpp -o juego_server.exe -lws2_32

g++ -std=c++17 -O2 -Wall -static -Iinclude src/juego_cliente_main.cpp src/juego_cliente.cpp src/juego_config.cpp src/red.cpp src/env.cpp -o juego_cliente.exe -lws2_32
```

> Los programas buscan el archivo `.env` en la carpeta actual. Si no lo encuentran allí (por ejemplo al ejecutar `../SistOpe/sistope.exe` desde otra carpeta), usan automáticamente la carpeta donde está el ejecutable, de modo que `.env`, `USUARIOS.TXT`, `PERFILES.TXT`, `LIBROS/` y los demás archivos se encuentran igual (se muestra el aviso `Nota: se usa la carpeta del programa: ...`). El archivo indicado con `-f` conserva su ubicación original.

---

## 5. Ejecución

### 5.1 Programa principal

```bash
./sistope.exe -u lve -p 123456 -f "LIBROS/drama_don_quijote_de_la_mancha_cervantes.txt"   # ADMIN
./sistope.exe -u ma  -p qwerty                                                            # GENERAL
```
(En Windows: `.\sistope.exe -u lve -p 123456` `.\sistope.exe -u Alphive -p 12345`.)

Cada programa también puede ejecutarse directamente, como en su entrega original:

```bash
./admin_usuarios.exe                      # Entrega 1 (sin argumentos, o con:  lve ADMIN)
./multi.exe a.txt b.txt "#" lve ADMIN     # Entrega 2
./indice_invertido.exe lve ADMIN          # Entrega 3 - Problema 1
./juego_server.exe   /   ./juego_cliente.exe maria    # Entrega 3 - Problema 2
```

### 5.2 Crear el índice invertido (Problema 1)

1. Iniciar `sistope.exe` con un usuario válido.
2. Elegir **8) Crear indice invertido**: el menú ejecuta `indice_invertido.exe` (llamada a sistema) y en ese programa se elige **1) Crear indice invertido**. Al salir con 0 se vuelve al menú principal.
3. El programa muestra la configuración leída, valida la carpeta, informa los libros identificados, el avance de cada thread, la verificación de finalización y un resumen (libros por thread, palabras, tiempo).
4. Se generan `indice_invertido.idx` y `libros_mapeo.txt` (según las variables de entorno).
5. La opción **2) Buscar una palabra en el indice generado** permite comprobar el resultado: muestra la línea del índice y traduce cada ID al nombre del libro usando el archivo de mapeo.

### 5.3 Jugar (Problema 2)

**Paso 1 – Iniciar el servidor** (una sola vez, en una terminal propia):

```bash
./juego_server.exe            # Linux / WSL
.\juego_server.exe            # Windows
```
También puede iniciarse desde el menú: **3) Juego → 2) Iniciar el servidor del juego** (en Windows se abre en una ventana nueva; en Linux queda en segundo plano y su actividad se ve en `juego.log`).

**Paso 2 – Conectar los jugadores** (una terminal por jugador):

```bash
./sistope.exe -u lve -p 123456     # y elegir 3) Juego -> 1) Unirse a una partida (juega con su username)
./juego_cliente.exe maria          # cliente independiente con el nombre indicado
./juego_cliente.exe pedro
```
Para jugar desde otros computadores de la red, en el `.env` de cada cliente se cambia `GAME_SERVER_IP` por la IP del computador que ejecuta el servidor (el servidor escucha en todas sus interfaces).

**Paso 3 – En el cliente** se escriben los comandos:

| Comando | Acción |
| :--- | :--- |
| `equipo A` (o `e a`) | unirse (o cambiarse) a EquipoA, EquipoB, … (solo antes de iniciar) |
| `listo` | confirmar que está listo para comenzar |
| `lanzar` (o `l`, o simplemente **ENTER** en su turno) | lanzar el dado |
| `chat <mensaje>` | mensaje para todos los jugadores |
| `estado` | ver equipos, posiciones y turno actual |
| `ayuda` | ver los comandos |
| `salir` | abandonar la partida |

Si el nombre ya está en uso en la partida (o no es válido), el servidor lo informa y el cliente pide otro nombre.

---

## 6. Problema 1 – Índice invertido concurrente

### 6.1 Flujo: cola → threads → semáforos → resultados → índice invertido

```
 [thread principal]                                  [N_THREADS threads trabajadores]
  1. valida variables de entorno
  2. verifica BOOKS_DIR (existe, es carpeta, se
     puede leer) e identifica los .txt legibles
  3. asigna ID 1..N (orden alfabético) y guarda
     BOOK_MAP_FILE   ->  1;biografias_...txt
  4. crea N_THREADS threads  ───────────────────────►  quedan BLOQUEADOS en la cola
  5. PRODUCTOR: encola cada libro ─────┐                 (semáforo contador en 0)
  6. encola N "píldoras de fin"        │
                                       ▼
                        ┌──────────── COLA DE TRABAJO ────────────┐
                        │ libro1 libro2 ... libroN FIN FIN ... FIN │
                        └──────────────────────────────────────────┘
                                       │  desencolar(): P(semLibrosDisponibles)
                                       │                P(semMutexCola) pop V(semMutexCola)
                                       ▼
                         cada thread trabajador:
                           a) obtiene UN libro de la cola
                           b) lee el archivo completo
                           c) cuenta palabras en un mapa LOCAL (paralelo, sin bloqueo)
                           d) P(semIndice) integra su resultado al índice global V(semIndice)
                           e) vuelve a (a) hasta recibir la píldora de fin
  7. join() de todos los threads
  8. verifica: cola vacía, N libros procesados, N_THREADS threads terminados
  9. ordena palabras y apariciones y escribe INVERTED_INDEX_FILE
```

### 6.2 Cola de trabajo

`ColaTrabajo` (en `indice_invertido.cpp`) contiene una `std::queue<LibroItem>`. Cada elemento representa **un libro pendiente** (`id`, nombre y ruta). Es un esquema **productor–consumidor**: los threads se crean *antes* de llenar la cola, por lo que realmente esperan y consumen mientras el productor agrega libros. Cuando un thread recibe un elemento con `id = 0` (píldora de fin) sabe que no quedan libros y termina; como la cola es FIFO, las píldoras siempre salen después de todos los libros.

### 6.3 Threads trabajadores

Se crean exactamente `N_THREADS` threads (`std::thread`). Cada uno ejecuta `threadTrabajador()`: obtener libro → procesar contenido → contar palabras → incorporar resultados al índice → pedir otro libro. Como el libro se retira de la cola dentro de la sección crítica, **un mismo libro nunca es procesado por dos threads**. Al final se informa cuántos libros procesó cada thread.

### 6.4 Semáforos y secciones críticas

Se implementó la clase `Semaforo` (`include/semaforo.h`) sobre los semáforos nativos del sistema operativo (POSIX `sem_init/sem_wait/sem_post` en Linux, `CreateSemaphore/WaitForSingleObject/ReleaseSemaphore` en Windows) con las operaciones **P = `esperar()`** y **V = `senalar()`**. La clase `SeccionCritica` hace P al entrar y V al salir (incluso si ocurre una excepción).

| Semáforo | Tipo / valor inicial | Recurso compartido protegido | Condición de carrera que evita |
| :--- | :--- | :--- | :--- |
| `semMutexCola` | binario / 1 | la `std::queue` de la cola de trabajo | dos threads haciendo `pop()` del mismo libro o corrompiendo la cola mientras el productor hace `push()` |
| `semLibrosDisponibles` | contador / 0 | cantidad de elementos en la cola | que un thread intente sacar de una cola vacía; los threads sin trabajo se bloquean sin consumir CPU |
| `semIndice` | binario / 1 | índice invertido global (`unordered_map`), contadores de libros procesados, total de palabras y lista de errores | inserciones simultáneas en el mapa (rehash) y pérdida de actualizaciones de contadores |
| `semConsola` | binario / 1 | salida por pantalla | mensajes de distintos threads mezclados en la misma línea |

El conteo de palabras de cada libro se realiza en un mapa **local** del thread (no compartido), por eso esa parte —la más costosa— corre en paralelo sin sincronización; solo la integración al índice global es sección crítica. El arreglo `librosPorThread` no requiere semáforo porque cada thread escribe únicamente su propia posición.

### 6.5 Conteo de palabras

Una palabra es una secuencia de letras: `a–z`, `A–Z` y letras latinas acentuadas (`á é í ó ú ü ñ`, etc.). Todo lo demás (espacios, números, puntuación, comillas, guiones) separa palabras. Las palabras se guardan en **minúsculas** y en UTF‑8, por lo que `Corazón`, `CORAZÓN` y `corazón` son la misma palabra. Se aceptan libros en UTF‑8 y también en Latin‑1.

### 6.6 Formato de los archivos generados

`BOOK_MAP_FILE` (asociación ID ↔ libro, IDs asignados en orden alfabético del nombre de archivo):
```
1;biografias_cronica_de_la_nueva_espana_cervantes_salazar.txt
2;biografias_historia_de_la_vida_del_buscon_quevedo.txt
...
```

`INVERTED_INDEX_FILE` (una línea por palabra, ordenadas alfabéticamente; cada palabra aparece **una sola vez** con todos los libros donde se encontró, ordenados por ID):
```
palabra;(id_libro,cantidad);(id_libro,cantidad);...
quijote;(16,2245);(38,2);(40,2);(43,12);...
```
`quijote;(16,2245);…` significa que *quijote* aparece 2245 veces en el libro 16 (`drama_don_quijote_de_la_mancha_cervantes.txt`), 2 veces en el libro 38, etc.

### 6.7 Finalización

El thread principal hace `join()` de **todos** los threads y luego verifica e informa: libros pendientes en la cola = 0, libros procesados = libros encontrados, threads finalizados = `N_THREADS`. Solo entonces escribe el índice. Si algo no cuadra, se informa el error y no se genera un índice incompleto. El programa nunca termina dejando threads activos ni trabajo en la cola.

### 6.8 Validaciones y errores informados

- Variable faltante (`INVERTED_INDEX_FILE`, `BOOK_MAP_FILE`, `BOOKS_DIR`, `N_THREADS`).
- `N_THREADS` no numérico, negativo, decimal, 0 o mayor a 1024.
- Carpeta inexistente, que no es carpeta, sin permisos de lectura, sin archivos `.txt` o con libros ilegibles (se informan y se omiten).
- Archivo de índice o de mapeo que no puede escribirse, o ambos apuntando al mismo archivo.

---

## 7. Problema 2 – Juego multiplayer "Carrera de Equipos"

### 7.1 Arquitectura: clientes → sockets → servidor → estado compartido

```
 sistope.exe (op. 3) ─┐  (ejecuta juego_cliente.exe)                    ┌──────────────── juego_server.exe ────────────────┐
 juego_cliente.exe  ──┼── TCP (sockets) ──►│ thread principal: accept() de conexiones          │
 juego_cliente.exe  ──┘   líneas de texto  │ 1 thread por cliente: lee y aplica sus comandos   │
                                           │ thread temporizador: tiempo máximo de cada turno  │
                                           │                                                   │
                                           │  ESTADO COMPARTIDO (protegido por un mutex):      │
                                           │  jugadores, equipos, posiciones, fase, turno      │
                                           │                                                   │
                                           │  broadcast de cada cambio ──► todos los clientes  │
                                           │  registro de cada evento  ──► GAME_LOG_FILE       │
                                           └───────────────────────────────────────────────────┘
```

- **Servidor (`juego_server.exe`)**: es la única autoridad. Mantiene el estado de la partida, lanza los dados (aleatorio 1..R), valida los turnos y difunde los cambios a todos los jugadores. Cada cliente es atendido por su propio thread; todo acceso al estado ocurre dentro de la sección crítica `mtxEstado`, de modo que dos jugadores nunca modifican la partida a la vez. El log tiene su propio mutex.
- **Cliente**: usa dos threads, uno **receptor** que muestra en tiempo real lo que envía el servidor y otro que lee el teclado y envía los comandos. Es el programa `juego_cliente.exe <nombre>`; `sistope.exe` lo ejecuta en la opción 3 → 1 mediante una llamada a sistema, pasándole el username de la sesión.
- **Sockets**: `red.cpp` encapsula sockets TCP de Berkeley (Linux) y Winsock2 (Windows) con la misma interfaz. Como TCP es un flujo de bytes, `LectorLineas` arma los mensajes completos (un mensaje = una línea terminada en `\n`).

### 7.2 Protocolo de mensajes

**Cliente → servidor**: `HOLA <nombre>`, `EQUIPO <letra>`, `LISTO`, `LANZAR`, `CHAT <texto>`, `ESTADO`, `AYUDA`, `SALIR`.

**Servidor → cliente** (la primera palabra indica el tipo): `INFO`, `ERROR`, `ERROR_NOMBRE`, `BIENVENIDO`, `ESPERA` (estado de la sala de espera), `INICIO`, `TURNO` (turno de un jugador, a todos), `TU_TURNO` (solo al jugador que debe lanzar), `RESULTADO`, `EVENTO`, `TABLERO`, `CHAT`, `ESTADO`, `AYUDA`, `FIN`.

Ejemplo de lo que ven todos los jugadores en un turno:
```
[TURNO] Turno 7 -> Turno de jugador: pedro (EquipoA, posicion 13)
[RESULTADO] pedro obtuvo: 4
[EVENTO] EquipoA avanzo desde la posicion 13 hasta la posicion 17.
[TABLERO] EquipoA=17 | EquipoB=12  (meta 30)
[TURNO] Turno 8 -> Turno de jugador: maria (EquipoB, posicion 12)
```

### 7.3 Reglas del juego

| Regla | Definición (valores del `.env` entregado) |
| :--- | :--- |
| **Número de jugadores** | mínimo `GAME_MIN_PLAYERS` = 2, máximo `GAME_MAX_PLAYERS` = 8. Conexiones por sobre el máximo se rechazan ("sala llena"). Cada jugador se identifica con un nombre único (1–20 caracteres: letras, números, `_ - .`). |
| **Equipos** | existen `GAME_MAX_TEAMS` = 4 equipos (EquipoA … EquipoD); para jugar se necesitan al menos `GAME_MIN_TEAMS` = 2 equipos con jugadores. Cada equipo admite de `GAME_MIN_PLAYERS_PER_TEAM` = 1 a `GAME_MAX_PLAYERS_PER_TEAM` = 2 jugadores. Los integrantes de un equipo comparten una ficha (posición) en el tablero. |
| **Condición de inicio** | la partida comienza automáticamente cuando: (1) hay al menos `GAME_MIN_PLAYERS` jugadores, (2) todos pertenecen a un equipo, (3) hay al menos `GAME_MIN_TEAMS` equipos con jugadores, (4) cada equipo con jugadores tiene al menos `GAME_MIN_PLAYERS_PER_TEAM`, y (5) **todos** escribieron `LISTO`. Mientras no se cumpla, cada cambio en la sala genera un mensaje `ESPERA` que indica exactamente qué falta. Una vez iniciada no se aceptan nuevos jugadores. |
| **Turnos** | el servidor alterna los equipos en orden (A → B → C → …) y, dentro de cada equipo, alterna a sus integrantes. Solo el jugador con el turno puede lanzar; cualquier otra acción fuera de lugar se rechaza con `ERROR` y queda en el log (`ACCION_RECHAZADA`). Si el jugador no lanza en `GAME_TURN_TIMEOUT` = 60 s, el servidor lanza por él (`TIEMPO_AGOTADO`). |
| **Acciones permitidas** | antes de iniciar: `EQUIPO`, `LISTO`; durante la partida: `LANZAR` (solo en su turno); siempre: `CHAT`, `ESTADO`, `AYUDA`, `SALIR`. |
| **Mecánica** | tablero de `GAME_BOARD_SIZE` = 30 posiciones; todos los equipos parten en la posición 0. Al lanzar un dado de `GAME_DICE_FACES` = 6 caras (valor aleatorio 1..R) el equipo avanza esa cantidad. |
| **Interacción (choque)** | si un equipo cae **exactamente** en la casilla donde está un rival, el rival **retrocede** tantas posiciones como el valor del dado (mínimo 0). |
| **Condición de término** | gana el primer equipo que **alcanza o supera** la posición `GAME_BOARD_SIZE`. También termina si, por desconexiones, queda un solo equipo con jugadores (gana por abandono) o ninguno (sin ganador). Al terminar, el servidor envía a todos el ganador y la tabla de posiciones, y realiza el cierre controlado. |

### 7.4 Desconexiones

El servidor detecta la desconexión (cierre del cliente, `SALIR`, caída de la conexión) cuando `recv()` devuelve 0 o error. Nunca provoca la caída del servidor ni de los otros jugadores: se registra `DESCONECTA` en el log, se avisa al resto y:
- en la sala de espera, el jugador sale de su equipo y se vuelve a evaluar la condición de inicio;
- durante la partida, sale de su equipo; si era su turno, el turno pasa al siguiente; si su equipo quedó sin jugadores, el equipo queda eliminado (`EQUIPO_ELIMINADO`); si queda un solo equipo, ese equipo gana.

### 7.5 Archivo de log (`GAME_LOG_FILE`)

Se abre en modo *append* (conserva las partidas anteriores). Formato **fijo de 5 campos** durante toda la ejecución:

```
fecha y hora;jugador;equipo;ACCION;detalle
```
Un campo que no aplica se registra como `-`; el detalle usa pares `clave=valor` separados por coma. Ejemplo (extracto de una partida):

```
2026-10-21 17:07:26;-;-;SERVIDOR_INICIA;puerto=8080,tablero=30,dado=6,jugadores=2-8,equipos=2-4,por_equipo=1-2
2026-10-21 17:07:26;ana;-;CONECTA;ip=127.0.0.1
2026-10-21 17:07:27;ana;EquipoA;UNE_EQUIPO;integrantes=1
2026-10-21 17:07:27;ana;EquipoA;LISTO;-
2026-10-21 17:07:28;-;-;INICIA_PARTIDA;jugadores=3,equipos=2,tablero=30,dado=6
2026-10-21 17:07:28;ana;EquipoA;TURNO;numero=1,posicion_equipo=0
2026-10-21 17:07:28;ana;EquipoA;LANZA_DADO;valor=4
2026-10-21 17:07:28;ana;EquipoA;AVANZA;desde=0,hasta=4
2026-10-21 17:07:29;ana;EquipoA;GANA;motivo=meta_alcanzada,posicion=30
2026-10-21 17:07:29;-;-;FIN_PARTIDA;motivo=meta_alcanzada,turnos=13
2026-10-21 17:07:29;-;EquipoA;RESULTADO;lugar=1,posicion=30
2026-10-21 17:07:29;beto;EquipoB;DESCONECTA;motivo=cierre_servidor
2026-10-21 17:07:29;-;-;SERVIDOR_CIERRA;conexiones_atendidas=3,threads_finalizados=4
```

| Evento exigido | Acción registrada |
| :--- | :--- |
| conexión de un jugador | `CONECTA` (y `RECHAZADO` si la sala está llena, la partida ya empezó o el nombre está en uso) |
| incorporación a un equipo | `UNE_EQUIPO` |
| inicio de la partida | `INICIA_PARTIDA` (además `LISTO` de cada jugador) |
| inicio o cambio de turno | `TURNO` |
| acción realizada por un jugador | `LANZA_DADO`, `CHAT`, `ACCION_RECHAZADA`, `TIEMPO_AGOTADO` |
| resultado de la acción | `AVANZA` (desde/hasta) |
| modificación relevante del estado | `CHOQUE`, `EQUIPO_ELIMINADO` |
| término de la partida | `FIN_PARTIDA` |
| resultado final / equipo ganador | `GANA`, `RESULTADO` (lugar y posición de cada equipo) |
| desconexión de jugadores | `DESCONECTA` (motivo: `cierre_cliente`, `salida_voluntaria`, `conexion_perdida`, `cierre_servidor`) |
| adicionales | `SERVIDOR_INICIA`, `SERVIDOR_INTERRUMPIDO` (Ctrl+C), `SERVIDOR_CIERRA` |

### 7.6 Cierre del servidor

Al terminar la partida (o con **Ctrl+C**) el servidor realiza un cierre controlado:
1. envía el resultado final a todos los jugadores;
2. **sockets**: interrumpe (`shutdown`) la conexión de cada cliente;
3. **threads**: espera con `join()` a todos los threads de clientes y al temporizador;
4. libera los descriptores (`close`/`closesocket`), el socket de escucha y Winsock (`WSACleanup`);
5. **estructuras compartidas**: vacía la lista de jugadores y equipos;
6. **archivo**: registra `SERVIDOR_CIERRA` y cierra el log.

---

## 8. Pruebas realizadas

- Índice invertido con `N_THREADS` = 1, 4 y 8: el archivo generado es **idéntico** en los tres casos y coincide byte a byte con una implementación de referencia independiente (97 libros, 9.360.123 palabras, 296.058 palabras distintas).
- Ejecución del índice y del servidor con **ThreadSanitizer** (`-fsanitize=thread`): sin condiciones de carrera reportadas.
- Validaciones de variables de entorno (faltantes, no numéricas, fuera de rango) y de la carpeta de libros (inexistente, vacía, sin permisos).
- Partidas completas con 2 y 3 jugadores (clientes `sistope.exe` y `juego_cliente.exe`): nombre duplicado, equipo completo, sala llena, intento de jugar fuera de turno o antes de iniciar, turno expirado, desconexión abrupta en pleno turno, victoria por abandono y cierre con Ctrl+C.
- Compilación sin advertencias (`-Wall -Wextra`) en Linux (g++ 13) y Windows (MinGW‑w64), con pruebas de los ejecutables de Windows.
- Opciones 1, 2, 4, 5, 6 y 7 de entregas anteriores verificadas nuevamente.
- Cadena de llamadas a sistema probada en una terminal real: `sistope.exe` → `admin_usuarios.exe` (un usuario creado allí puede luego iniciar sesión), → `multi.exe`, → `indice_invertido.exe`, → `juego_server.exe` + `juego_cliente.exe` (partida completa y regreso al menú). En Windows se verificaron las comillas de los argumentos (incluso con espacios) y los códigos de salida.

---

## 9. Funcionalidades de entregas anteriores (siguen operativas)

1. **Autenticación por línea de comandos**: `-u <usuario>`, `-p <password>`, `-f <archivo>`. Sin credenciales válidas no se muestra el menú.
2. **Encabezado de sesión** `User: <username> (<perfil>)` en el menú.
3. **Control de acceso por perfil**: cada perfil define las opciones permitidas; la opción 1 es exclusiva de `ADMIN`.
3b. **Administración de usuarios y perfiles (Entrega 1)**: programa `admin_usuarios.exe`, llamado desde la opción 1.
4. **Persistencia binaria** de `Usuario` (144 bytes) y `Perfil` (116 bytes) como structs de tamaño fijo, leídos y escritos en un solo bloque con `read((char*)&s, sizeof(s))` / `write((const char*)&s, sizeof(s))`. El script `tools/migrar_txt_a_dat.py` convierte archivos de texto antiguos (`id;nombre;...`) a este formato o regenera los `.DAT`; el programa no lo necesita para funcionar.
5. **`multi.exe`**: `./multi.exe a.txt b.txt "#" lve ADMIN` multiplica matrices N×M validando archivos, formato y dimensiones. El menú lo invoca como programa externo mediante una llamada a sistema (también funciona en Linux).
6. **Conteo** de vocales, consonantes, caracteres especiales y palabras (opciones 6 y 7).

Usuarios de prueba: `lve / 123456` (ADMIN) y `ma / qwerty` (GENERAL).
