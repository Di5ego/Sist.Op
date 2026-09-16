#include "../include/cli.h"
#include <iostream>
#include <algorithm>

using namespace std;

// Procesa las banderas de la linea de comandos (-u, -p, -f) y realiza
// la autenticacion del usuario contra la base de datos de usuarios y perfiles.
SesionUsuario procesarArgumentosCLI(int argc, char* argv[], ListaUsuarios& listaUsuarios, ListaPerfiles& listaPerfiles, const string& rutaUsuarios, const string& rutaPerfiles) {
    SesionUsuario sesion;
    string usernameOpt = "";
    string passwordOpt = "";
    string fileOpt = "";

    // Extraer argumentos pasados por consola
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "-u" && i + 1 < argc) {
            usernameOpt = argv[++i];
        } else if (arg == "-p" && i + 1 < argc) {
            passwordOpt = argv[++i];
        } else if (arg == "-f" && i + 1 < argc) {
            fileOpt = argv[++i];
        }
    }

    // Validar presencia de credenciales obligatorias
    if (usernameOpt.empty() || passwordOpt.empty()) {
        cerr << "Error: Argumentos de ejecucion insuficientes.\n";
        cerr << "Uso requerido: ./sistope -u <usuario> -p <password> [-f <archivo>]\n";
        cerr << "Ejemplo: ./sistope -u lve -p 123456 -f \"LIBROS/libro1.txt\"\n";
        return sesion;
    }

    if (!listaUsuarios.cargadoEnMemoria) {
        cargarUsuariosDesdeArchivo(listaUsuarios, rutaUsuarios);
    }
    if (!listaPerfiles.cargadoEnMemoria) {
        cargarPerfilesDesdeArchivo(listaPerfiles, rutaPerfiles);
    }

    // Buscar usuario y validar coincidencia de clave
    auto itU = find_if(listaUsuarios.usuarios.begin(), listaUsuarios.usuarios.end(), [&usernameOpt, &passwordOpt](const Usuario& u) {
        return u.username == usernameOpt && u.password == passwordOpt;
    });

    if (itU == listaUsuarios.usuarios.end()) {
        cerr << "Error de autenticacion: Usuario '" << usernameOpt << "' o contrasena invalida.\n";
        return sesion;
    }

    sesion.autenticado = true;
    sesion.usuario = *itU;
    sesion.archivoPorDefecto = fileOpt;

    // Obtener permisos y opciones del perfil asociado al usuario
    auto itP = find_if(listaPerfiles.perfiles.begin(), listaPerfiles.perfiles.end(), [&sesion](const Perfil& p) {
        return p.nombre == sesion.usuario.perfil;
    });

    if (itP != listaPerfiles.perfiles.end()) {
        sesion.perfil = *itP;
    } else {
        sesion.perfil.nombre = sesion.usuario.perfil;
    }

    return sesion;
}
