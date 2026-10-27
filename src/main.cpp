#include <iostream>
#include <string>
#include <vector>

#include "../include/usuario.h"
#include "../include/perfil.h"
#include "../include/menu.h"
#include "../include/env.h"
#include "../include/cli.h"

using namespace std;

// Punto de entrada principal
// Carga variables de entorno, autentica al usuario mediante argumentos CLI (-u, -p, -f)
// y despliega el Menú Principal con las opciones requeridas (Entregas 2 y 3).
int main(int argc, char* argv[]) {
    // 0. Si se ejecuta desde otra carpeta, se ubica la carpeta del programa
    //    (donde estan .env, USUARIOS.TXT, LIBROS/, ...). Antes se convierte
    //    a ruta absoluta el archivo de -f para no perder su ubicacion original.
    vector<string> argumentos(argv, argv + argc);
    for (int i = 1; i + 1 < argc; ++i) {
        if (argumentos[i] == "-f") argumentos[i + 1] = rutaAbsoluta(argumentos[i + 1]);
    }
    string carpetaPrograma;
    bool cambioCarpeta = ubicarCarpetaDelPrograma(argc > 0 ? argv[0] : nullptr, carpetaPrograma);
    vector<char*> argvAjustado;
    for (auto& a : argumentos) argvAjustado.push_back(&a[0]);
    argvAjustado.push_back(nullptr);
    if (cambioCarpeta) {
        argv = argvAjustado.data();
        cout << "Nota: se usa la carpeta del programa: " << carpetaPrograma << "\n";
    }

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
