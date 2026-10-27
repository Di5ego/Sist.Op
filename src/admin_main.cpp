#include "../include/admin_usuarios.h"
#include "../include/env.h"

#include <iostream>
#include <string>

using namespace std;

int main(int argc, char* argv[]) {
    string carpetaPrograma;
    ubicarCarpetaDelPrograma(argc > 0 ? argv[0] : nullptr, carpetaPrograma);
    cargarVariablesEntorno(".env");

    string username = argc >= 2 ? argv[1] : "";
    string perfil = argc >= 3 ? argv[2] : "";

    // Si lo llama el menu principal, se informa la sesion y se protege la
    // integridad del sistema: solo el perfil ADMIN puede administrar.
    if (!perfil.empty() && perfil != "ADMIN") {
        cerr << "Acceso denegado: el perfil '" << perfil << "' no puede administrar usuarios y perfiles.\n";
        return 2;
    }

    string rutaUsuarios = obtenerVariableEntorno("USER_FILE");
    string rutaPerfiles = obtenerVariableEntorno("PERFIL_FILE");
    if (rutaUsuarios.empty() || rutaPerfiles.empty()) {
        cerr << "Error: las variables de entorno USER_FILE y PERFIL_FILE deben estar definidas en el .env.\n";
        return 1;
    }

    cout << "============================================================\n";
    cout << "   SISTOPE - ADMINISTRACION DE USUARIOS Y PERFILES (Entrega 1)\n";
    if (!username.empty()) cout << "User: " << username << " (" << perfil << ")\n";
    cout << "============================================================\n";

    ListaUsuarios listaUsuarios;
    ListaPerfiles listaPerfiles;
    ejecutarAdministradorUsuariosYPerfiles(listaUsuarios, listaPerfiles, rutaUsuarios, rutaPerfiles);
    return 0;
}
