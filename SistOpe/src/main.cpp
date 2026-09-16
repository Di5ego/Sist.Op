#include <iostream>
#include <string>

#include "../include/usuario.h"
#include "../include/perfil.h"
#include "../include/menu.h"
#include "../include/env.h"
#include "../include/cli.h"

using namespace std;

// Punto de entrada principal
// Carga variables de entorno, autentica al usuario mediante argumentos CLI (-u, -p, -f)
// y despliega el Menú Principal con las 7 opciones requeridas.
int main(int argc, char* argv[]) {
    // 1. Cargar variables de entorno desde el archivo .env
    cargarVariablesEntorno(".env");
    
    string userFile = obtenerVariableEntorno("USER_FILE", "USUARIOS.TXT");
    string perfilFile = obtenerVariableEntorno("PERFIL_FILE", "PERFILES.TXT");
    string defaultTextFile = obtenerVariableEntorno("DEFAULT_TEXT_FILE", "LIBROS/drama_don_quijote_de_la_mancha_cervantes.txt");

    // 2. Inicializar estructuras de memoria
    ListaUsuarios listaUsuarios;
    ListaPerfiles listaPerfiles;

    // 3. Procesar banderas de linea de comandos (-u, -p, -f) y autenticar
    SesionUsuario sesion = procesarArgumentosCLI(argc, argv, listaUsuarios, listaPerfiles, userFile, perfilFile);

    if (!sesion.autenticado) {
        cerr << "\nAcceso denegado. El sistema requiere autenticacion valida para iniciar.\n";
        return 1;
    }
    
    if (sesion.archivoPorDefecto.empty()) {
        sesion.archivoPorDefecto = defaultTextFile;
    }

    cout << "\n============================================================\n";
    cout << "  Bienvenido " << sesion.usuario.nombre << " [" << sesion.usuario.perfil << "]\n";
    cout << "============================================================\n";

    // 4. Iniciar el Menu Principal de Entrega 2
    mostrarMenuPrincipalEntrega2(sesion, listaUsuarios, listaPerfiles, userFile, perfilFile);

    return 0;
}
