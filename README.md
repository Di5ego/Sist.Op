# SistOpe - Sistema de Administración de Usuarios, Perfiles y Menú Principal (Entrega 2)

**Asignatura:** Sistemas Operativos (INFO198)  
**Profesor:** Luis Veas-Castillo  
**Entrega:** 2  

---

## 1. Propósito de la Aplicación

**SistOpe** es un sistema modular desarrollado en C++ para la asignatura de Sistemas Operativos. En esta **Entrega 2**, la aplicación incorpora autenticación obligatoria por argumentos en línea de comandos (`-u`, `-p`, `-f`), control de acceso estricto basado en perfiles (`ADMIN`), e integra un menú principal de 7 opciones funcionales.

### Características Principales (Entrega 2):
1. **Autenticación e Invocación por Banderas CLI:**
   - `-u <usuario>`: Especifica el nombre de usuario (`username`).
   - `-p <password>`: Especifica la contraseña.
   - `-f <archivo>`: Especifica la ruta por defecto para el conteo de texto.
   - Si las credenciales no son válidas o faltan argumentos obligatorios, el sistema deniega el acceso antes de mostrar el menú.

2. **Encabezado Informativo de Sesión:**
   Muestra constantemente en pantalla las credenciales del usuario autenticado:  
   `User: <username> (<perfil>)` (ej: `User: lve (ADMIN)`).

3. **Menú Principal de 7 Opciones + Salir (0):**
   - **`0) Salir`**: Finaliza el programa de forma segura.
   - **`1) Admin de usuarios y perfiles`**: Accesible **únicamente** por usuarios con perfil `ADMIN`. Invoca el sistema completo de gestión de usuarios y perfiles de la Entrega 1.
   - **`2) Multiplica matrices NxM`**: Ejecuta el programa independiente `multi.exe` para multiplicar matrices cuadradas o rectangulares $N \times M$ validadas desde archivos de texto.
   - **`3) Juego`**: Despliega la pantalla informativa `"(mensaje en construcción)"`.
   - **`4) ¿es palíndromo?`**: Submenú interactivo para ingresar texto y validar si es un palíndromo (ignorando mayúsculas, minúsculas, espacios y caracteres especiales).
   - **`5) Calcular f(x)=x*x + 2x + 8`**: Calcula la función $f(x) = x^2 + 2x + 8$ aceptando números reales decimales (`double`).
   - **`6) CONTEO SOBRE TEXTO`**: Procesa el archivo ingresado mediante la bandera `-f` y entrega un resumen con: cantidad de vocales, consonantes, caracteres especiales y palabras.
   - **`7) CONTEO SOBRE ARCHIVO`**: Interfaz interactiva que solicita la ruta de cualquier archivo e impresiona el mismo resumen de conteo que la opción 6.

4. **Programa Independiente Multiplicador de Matrices (`multi.exe`):**
   - Recibe argumentos CLI: `./multi <ruta_A.TXT> <ruta_B.TXT> <separador> [username] [perfil]`.
   - Valida la existencia de los archivos, formato, contenido numérico y compatibilidad de dimensiones ($N \times M$ con $M \times P$).

5. **Carpeta `LIBROS/` (> 50 MB):**
   - Incluye una colección de libros en formato `.txt` de diversos géneros (ciencia ficción, fantasía, drama, biografías, ciencias, naturaleza) superando un tamaño total de **50 MB** para pruebas masivas de conteo de texto.
   -Link libros para el ejecutable: https://drive.google.com/drive/folders/1LyTzwUJr2Pf6XJUQK9Msw97EC12I4_Ah?usp=drive_link
---

## 2. Estructura del Proyecto

```text
SistOpe/
  Makefile                    # Script de compilación automática
  README.md                   # Documentación oficial del proyecto
  USUARIOS.TXT                # Persistencia de usuarios
  PERFILES.TXT                # Persistencia de perfiles
  .env                        # Variables de entorno
  a.txt / b.txt               # Archivos de prueba para matrices
  sistope.exe                 # Ejecutable principal (Entrega 2)
  multi.exe                   # Ejecutable independiente multiplicador de matrices

  LIBROS/                     # Carpeta con >50MB de libros en .txt
    ciencia_ficcion.txt
    fantasia.txt
    drama.txt
    biografias.txt
    ciencias.txt
    naturaleza.txt

  include/                    # Archivos de cabecera (.h)
    cli.h
    conteo.h
    env.h
    matriz.h
    menu.h
    perfil.h
    usuario.h
    utilidades.h

  src/                        # Código fuente (.cpp)
    cli.cpp
    conteo.cpp
    env.cpp
    main.cpp
    matriz.cpp
    menu.cpp
    multi_main.cpp
    perfil.cpp
    usuario.cpp
    utilidades.cpp
```

---

## 3. Variables de Entorno (`.env`)

El sistema utiliza las siguientes variables de entorno para configurar las rutas por defecto:

| Variable | Descripción | Valor por Defecto |
| :--- | :--- | :--- |
| `USER_FILE` | Ruta del archivo de texto para Usuarios | `USUARIOS.TXT` |
| `PERFIL_FILE` | Ruta del archivo de texto para Perfiles | `PERFILES.TXT` |
| `DEFAULT_TEXT_FILE` | Archivo de texto por defecto para la bandera `-f` | `LIBROS/libro1.txt` |
| `MATRIX_EXEC` | Nombre del ejecutable multiplicador de matrices | `multi.exe` |

Contenido del archivo `.env`:
```env
USER_FILE=USUARIOS.TXT
PERFIL_FILE=PERFILES.TXT
DEFAULT_TEXT_FILE=LIBROS/ciencia_ficcion.txt
MATRIX_EXEC=multi.exe
```

---

## 4. Instrucciones de Compilación y Ejecución

### Prerrequisitos
- Compilador de C++ (`g++` compatible con C++17)
- Herramienta `make` (opcional en Linux/macOS)

---

### Compilación

#### Opción A: Usando `make` (Linux / macOS / WSL)
```bash
make
```

#### Opción B: Compilación directa con `g++` (Windows PowerShell / CMD)
```powershell
# Compilar ejecutable principal
g++ -std=c++17 -static-libgcc -static-libstdc++ -Wall -Iinclude src/main.cpp src/usuario.cpp src/perfil.cpp src/menu.cpp src/env.cpp src/cli.cpp src/conteo.cpp src/utilidades.cpp src/matriz.cpp -o sistope.exe

# Compilar ejecutable multiplicador de matrices
g++ -std=c++17 -static-libgcc -static-libstdc++ -Wall -Iinclude src/multi_main.cpp src/matriz.cpp -o multi.exe
```

---

### Ejecución

#### 1. Invocación del Menú Principal (sistope.exe)
Ejemplo de ejecución ingresando como Administrador:
```powershell
.\sistope.exe -u lve -p 123456 -f "LIBROS/ciencia_ficcion.txt"
```

Ejemplo de ejecución ingresando como Usuario General:
```powershell
.\sistope.exe -u ma -p qwerty -f "LIBROS/fantasia.txt"
```

#### 2. Invocación Directa del Multiplicador de Matrices (multi.exe)
```powershell
.\multi.exe "a.txt" "b.txt" "#" "lve" "ADMIN"
```
