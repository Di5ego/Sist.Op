# SistOpe - Administrador de Usuarios y Perfiles

**Asignatura:** Sistemas Operativos (INFO198)  
**Profesor:** Luis Veas-Castillo  
**Entrega:** 1  

---

## 1. Propósito de la Aplicación

Consiste en un **"Administrador de Usuarios y Perfiles"** desarrollado en c++. Realiza la gestion completa (Crear, Listar y Eliminar) de usuarios y perfiles del sistema.

El programa hace lo escencial: te permite crear, ver y eliminar usuarios o perfiles. Mientras usas la aplicacion, los datos se manejan rapido en la memoria usando `structs` (`struct Usuario`, `struct ListaUsuarios`, `struct Perfil`, `struct ListaPerfiles`), y al realizar un cambio todo queda guardado en archivos de texto. Tambien agregamos un control de permisos basico(ADMIN y GENERAL).


### Características Principales:
- **Estructuras de Datos (`struct`):**
  - **Usuario:** Contiene `id`, `nombre`, `username`, `password` y `perfil`.
  - **ListaUsuarios:** Mantiene el vector de usuarios en memoria.
  - **Perfil:** Contiene `nombre` y un array de enteros (`opciones`) que representan los permisos del menu.
  - **ListaPerfiles:** Mantiene el vector de perfiles en memoria.
- **Persistencia en Disco:**
  - Persistencia de usuarios en `USUARIOS.TXT` (formato `id;nombre;username;password;perfil`).
  - Persistencia de perfiles en `PERFILES.TXT` (formato `NOMBRE;opcion1,opcion2,...`).
- **Navegación e Identificadores:** Tratamiento interno numerico (`int`) de todas las opciones de menu. La opción `0` permite salir o volver
- **Alertas de Seguridad:** En la eliminacion de un usuario o perfil con rol `"ADMIN"`, el sistema muestra una advertencia de seguridad explícita antes de solicitar la confirmación (`1) guardar` / `2) cancelar`).

---

## 2. Estructura del Proyecto

```
SistOpTorre/
  Makefile                    
  README.md                   
  USUARIOS.TXT                
  PERFILES.TXT                
  .env                        

  include/                    
    menu.h                    
    perfil.h                  
    usuario.h                 

  src/                        
    main.cpp                  
    menu.cpp                  
    usuario.cpp               
    output/                   
```

### Descripción de Archivos Principales

#### **`include/` - Archivos de Cabecera**
- **`usuario.h`**: Declara la estructura `Usuario` y `ListaUsuarios`, junto con funciones para crear, listar, buscar y eliminar usuarios.
- **`perfil.h`**: Declara la estructura `Perfil` y `ListaPerfiles`, junto con funciones para gestionar perfiles.
- **`menu.h`**: Declara funciones para la interfaz de usuario (menús, entrada/salida).

#### **`src/` - Código Fuente**
- **`main.cpp`**: Contiene la función `main()` que inicializa la aplicación y controla el flujo principal del programa.
- **`usuario.cpp`**: Implementación de todas las operaciones con usuarios (crear, listar, modificar, eliminar).
- **`perfil.cpp`**: Implementación de todas las operaciones con perfiles.
- **`menu.cpp`**: Implementación de la interfaz interactiva.

#### **Archivos de Datos**
- **`.env`**: Contiene las variables de entorno
- **`USUARIOS.TXT`**: Almacena los usuarios registrados en formato texto. Se sincroniza automáticamente con la memoria.
- **`PERFILES.TXT`**: Almacena los perfiles disponibles en formato texto. Se sincroniza automáticamente con la memoria.

#### **`Makefile`**
Archivo de configuracion que automatiza la compilación del proyecto. Permite compilar todo con el comando `make`.

---

## 3. Descripción de las Variables de Entorno

El sistema puede leer variables de entorno definidas en un archivo `.env` .

| Variable | Descripcion | Valor por Defecto |
| :--- | :--- | :--- |
| `USER_FILE` | Ruta del archivo de texto para la persistencia de Usuarios | `USUARIOS.TXT` |
| `PERFIL_FILE` | Ruta del archivo de texto para la persistencia de Perfiles | `PERFILES.TXT` |

Contenido del archivo `.env`:
```env
USER_FILE=USUARIOS.TXT
PERFIL_FILE=PERFILES.TXT
```

---

## 4. Instrucciones de Compilación y Ejecución

### Prerrequisitos
- Compilador de C++ 
- Herramienta `make` 

### En Linux / macOS / WSL
```bash
# 1. Compilar el proyecto usando Makefile
make

# 2. Ejecutar la aplicación
./sistope
```

### En Windows (PowerShell / CMD / Command Prompt)
```cmd
# Compilación directa con g++
g++ -std=c++17 -static-libgcc -static-libstdc++ -Wall -Iinclude src/main.cpp src/usuario.cpp src/perfil.cpp src/menu.cpp -o sistope.exe

# Ejecución
sistope.exe
```

---

## 5. Flujo de Ejecución

1. **Inicialización:** El programa carga los usuarios y perfiles desde los archivos `USUARIOS.TXT` y `PERFILES.TXT`.
2. **Menú Principal:** Se presenta un menú interactivo con opciones para gestionar usuarios y perfiles.
3. **Operaciones CRUD:** El usuario puede crear, listar, modificar y eliminar usuarios y perfiles.
4. **Persistencia:** Cada cambio se sincroniza automáticamente con los archivos de datos.
5. **Salida:** Opción de salir del programa con la opción `0`.


