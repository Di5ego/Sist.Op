#ifndef EXTERNO_H
#define EXTERNO_H

// ============================================================================
// Llamada a sistema para ejecutar los programas de cada entrega
// ----------------------------------------------------------------------------
// El sistema SistOpe se construye por entregas y cada entrega reutiliza la
// anterior ejecutandola como un PROGRAMA EXTERNO (llamada a sistema), en vez
// de copiar su codigo:
//
//   sistope.exe (Entrega 2: MENU PRINCIPAL)
//     (1) --> admin_usuarios.exe   Entrega 1: Administracion de usuarios y perfiles
//     (2) --> multi.exe            Entrega 2: Multiplicador de matrices
//     (3) --> juego_cliente.exe    Entrega 3: Juego (cliente)   / juego_server.exe
//     (8) --> indice_invertido.exe Entrega 3: Indice invertido concurrente
//
// El nombre de cada ejecutable se obtiene desde una variable de entorno.
// ============================================================================

#include <string>
#include <vector>

// Ejecuta 'ejecutable' con los argumentos indicados mediante system().
// Cada argumento se entrega entre comillas (soporta espacios) en Windows (cmd)
// y en Linux (sh). Si el ejecutable no trae ruta, en Linux se busca en la
// carpeta actual (./programa.exe). Devuelve el codigo de salida del programa
// (0 = exito) o -1 si no fue posible ejecutarlo.
int ejecutarProgramaExterno(const std::string& ejecutable, const std::vector<std::string>& argumentos);

// Igual que la anterior, pero lee el nombre del ejecutable desde la variable
// de entorno 'variable'. Informa por pantalla si la variable no existe, si el
// ejecutable no se encuentra o si termino con error.
int llamarPrograma(const std::string& variable, const std::string& descripcion,
                   const std::vector<std::string>& argumentos);

#endif // EXTERNO_H
