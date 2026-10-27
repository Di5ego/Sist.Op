#ifndef CLI_H
#define CLI_H

#include <string>
#include "usuario.h"
#include "perfil.h"

// Estructura para almacenar la información de sesión del usuario
struct SesionUsuario {
    bool autenticado = false;
    Usuario usuario;
    Perfil perfil;
    std::string archivoPorDefecto = "";
};

// Parsea los argumentos CLI (-u, -p, -f) y autentica al usuario contra listaUsuarios y listaPerfiles
SesionUsuario procesarArgumentosCLI(int argc, char* argv[], ListaUsuarios& listaUsuarios, ListaPerfiles& listaPerfiles, const std::string& rutaUsuarios, const std::string& rutaPerfiles);

#endif // CLI_H
