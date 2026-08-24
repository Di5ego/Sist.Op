#include "../include/menu.h"
#include <iostream>
#include <limits>
#include <sstream>
#include <algorithm>

using namespace std;
// Funcion auxiliar para leer opcion entera con validacion 
static int leerOpcion() {
    int opcion = 0;
    cout << "\nOpcion : ";
    while (!(cin >> opcion)) {
        if (cin.eof()) return 0;
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Entrada invalida. Ingrese un numero para la opcion: ";
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');// Limpiar el buffer de entrada
    return opcion;
}

// Funcion auxiliar para leer cadenas con espacios
static string leerLinea(const string& prompt) {
    string input;
    cout << prompt;
    if (!getline(cin, input)) {
        return "";
    }
    return input;
}

// PANTALLAS DE GESTION DE USUARIOS

static void pantallaIngresarUsuario(ListaUsuarios& listaUsuarios, ListaPerfiles& listaPerfiles, const string& rutaUsuarios, const string& rutaPerfiles) {
    cout << "Ingreso de usuarios\n";

    if (!listaUsuarios.cargadoEnMemoria) {
        cargarUsuariosDesdeArchivo(listaUsuarios, rutaUsuarios);
    }
    if (!listaPerfiles.cargadoEnMemoria) {
        cargarPerfilesDesdeArchivo(listaPerfiles, rutaPerfiles);
    }

    Usuario u;
    
    // Validar Id usuario
    while (true) {
        string idStr = leerLinea("Id: ");
        if (idStr.empty()) {
            cout << "Error! El ID no puede estar vacio\n";
            continue;
        }
        try {
            u.id = stoi(idStr);
            if (u.id <= 0) {
                cout << "Error! El ID debe ser un numero entero positivo mayor a 0\n";
                continue;
            }
            if (existeUsuarioId(listaUsuarios, u.id)) {
                cout << "Error! Ya existe un usuario registrado con el ID " << u.id << ". Intente con otro\n";
                continue;
            }
            break;
        } catch (...) {
            cout << "Error! El ID debe ser un numero entero valido\n";
        }
    }

    // Validar Nombre (minimo 2 caracteres)
    while (true) {
        u.nombre = leerLinea("Nombre (Primer nombre y primer apellido): ");
        if (u.nombre.empty()) {
            cout << "Error! El nombre no puede estar vacio\n";
            continue;
        }
        if (u.nombre.length() < 5) {
            cout << "Error! El nombre debe tener un minimo de 5 caracteres.\n";
            continue;
        }
        break;
    }

    // Validar Username (minimo 2 caracteres y unico)
    while (true) {
        u.username = leerLinea("username: ");
        if (u.username.empty()) {
            cout << "Error! El username no puede estar vacio\n";
            continue;
        }
        if (u.username.length() < 2) {
            cout << "Error! El username debe tener un minimo de 2 caracteres\n";
            continue;
        }
        if (existeUsername(listaUsuarios, u.username)) {
            cout << "Error! El username '" << u.username << "' ya existe. Por favor elija uno diferente\n";
            continue;
        }
        break;
    }

    // Validar Password (mínimo 2 caracteres)
    while (true) {
        u.password = leerLinea("password: ");
        if (u.password.empty()) {
            cout << "Error! La contraseña no puede estar vacia\n";
            continue;
        }
        if (u.password.length() < 2) {
            cout << "Error! La contraseña debe tener un minimo de 2 caracteres\n";
            continue;
        }
        break;
    }
    
    // Validar Perfil (mínimo 2 caracteres)
    while (true) {
        u.perfil = leerLinea("perfil: ");
        if (u.perfil.empty()) {
            cout << "Error! El perfil no puede estar vacio\n";
            continue;
        }
        if (u.perfil.length() < 2) {
            cout << "Error! El nombre del perfil debe tener un minimo de 2 caracteres\n";
            continue;
        }
        if (!existePerfil(listaPerfiles, u.perfil)) {
            cout << "Error! El perfil '" << u.perfil << "' no se encuentra registrado en los perfiles del sistema.\n";
            continue;
        } else {
            break;
        }
    }

    cout << "\n1) guardar    2) cancelar\n";
    int op = leerOpcion();
    if (op == 1) {
        if (guardarUsuario(listaUsuarios, u, rutaUsuarios)) {
            cout << "\n-> Usuario guardado exitosamente.\n";
        } else {
            cout << "\n-> Error al guardar el usuario.\n";
        }
    } else {
        cout << "\n-> Operacion cancelada\n";
    }
}

static void pantallaListarUsuarios(ListaUsuarios& listaUsuarios, const string& rutaUsuarios) {
    listarUsuarios(listaUsuarios, rutaUsuarios);
    
    int op = -1;
    while (op != 1) {
        cout << "\n1) para Volver\n";
        op = leerOpcion();
    }
}

static void pantallaEliminarUsuario(ListaUsuarios& listaUsuarios, const string& rutaUsuarios) {
    cout << "\n------------------------------------------------------------\n";
    cout << "Eliminar Usuarios\n";
    cout << "------------------------------------------------------------\n";

    if (!listaUsuarios.cargadoEnMemoria) {
        cargarUsuariosDesdeArchivo(listaUsuarios, rutaUsuarios);
    }

    string idStr = leerLinea("ID usuario a borrar: ");
    int idBorrar = -1;
    try {
        idBorrar = stoi(idStr);
    } catch (...) {
        cout << "Error! ID invalido.\n";
        return;
    }

    // Buscar si el usuario existe y si su perfil es ADMIN
    auto it = find_if(listaUsuarios.usuarios.begin(), listaUsuarios.usuarios.end(), [idBorrar](const Usuario& u) {
        return u.id == idBorrar;
    });

    if (it == listaUsuarios.usuarios.end()) {
        cout << "Error! No existe ningun usuario registrado con el ID " << idBorrar << ".\n";
        return;
    }

    // Requerimiento: "si el usuario a eliminar es perfil 'ADMIN', debe enviar un mensaje de alerta señalando el posible error"
    if (it->perfil == "ADMIN") {
        cout << "ALERTA!: El usuario que intenta eliminar tiene perfil 'ADMIN'\n";
        cout << "Eliminar un usuario administrador puede provocar perdida de acceso\n";
    }

    cout << "\n1) guardar    2) cancelar\n";
    int op = leerOpcion();
    if (op == 1) {
        if (eliminarUsuarioPorId(listaUsuarios, idBorrar, rutaUsuarios, true)) {
            cout << "\n-> Usuario eliminado exitosamente\n";
        } else {
            cout << "\n-> No se pudo eliminar el usuario\n";
        }
    } else {
        cout << "\n-> Operacion cancelada\n";
    }
}

void menuGestionUsuarios(ListaUsuarios& listaUsuarios, ListaPerfiles& listaPerfiles, const string& rutaUsuarios, const string& rutaPerfiles) {
    int opcion = -1;
    while (opcion != 0) {
        cout << "Modulo - Gestion de Usuarios\n";
        cout << "0) Salir\n";
        cout << "1) Ingresar Usuarios\n";
        cout << "2) Listar Usuarios\n";
        cout << "3) Eliminar Usuarios\n";
        
        opcion = leerOpcion();

        switch (opcion) {
            case 0:
                break;
            case 1:
                pantallaIngresarUsuario(listaUsuarios, listaPerfiles, rutaUsuarios, rutaPerfiles);
                break;
            case 2:
                pantallaListarUsuarios(listaUsuarios, rutaUsuarios);
                break;
            case 3:
                pantallaEliminarUsuario(listaUsuarios, rutaUsuarios);
                break;
            default:
                cout << "Opcion invalida, intente de nuevo\n";
                break;
        }
    }
}

// PANTALLAS DE GESTION DE PERFILES

static void pantallaIngresarPerfil(ListaPerfiles& listaPerfiles, const string& rutaPerfiles) {
    cout << "Ingreso de perfiles\n";

    if (!listaPerfiles.cargadoEnMemoria) {
        cargarPerfilesDesdeArchivo(listaPerfiles, rutaPerfiles);
    }

    Perfil p;
    // Validar nombre de perfil minimo 2 caracteres y no duplicado
    while (true) {
        p.nombre = leerLinea("Nombre del perfil (ej: ADMIN, GENERAL: ");
        if (p.nombre.empty()) {
            cout << "Error! El nombre del perfil no puede estar vacio\n";
            continue;
        }
        if (p.nombre.length() < 2) {
            cout << "Error! El nombre del perfil debe tener un minimo de 2 caracteres\n";
            continue;
        }
        if (existePerfil(listaPerfiles, p.nombre)) {
            cout << "Error! Ya existe un perfil denominado '" << p.nombre << "'. Por favor elija un nombre diferente\n";
            continue;
        }
        break;
    }

    // Valida opciones del menu
    while (true) {
        string opsLine = leerLinea("Opciones de menu permitidas (separadas por coma, ej: 0,1,2,3): ");
        if (opsLine.empty()) {
            cout << "Error! Debe ingresar al menos una opcion de menu permitida\n";
            continue;
        }

        p.opciones.clear();
        stringstream ss(opsLine);// Separar por comas
        string token;
        while (getline(ss, token, ',')) {
            try {
                p.opciones.push_back(stoi(token));// Convertir a entero y agregar a la lista de opciones
            } catch (...) {
                // omitir espacios o caracteres invalidos
            }
        }

        if (p.opciones.empty()) {
            cout << "Error! No se reconocieron numeros de opcion validos, debe ingrese opciones numericas separadas por coma\n";
            continue;
        }
        break;
    }

    cout << "\n1) guardar    2) cancelar\n";
    int op = leerOpcion();
    if (op == 1) {
        if (guardarPerfil(listaPerfiles, p, rutaPerfiles)) {
            cout << "\n-> Perfil guardado exitosamente\n";
        } else {
            cout << "\n-> Error al guardar el perfil\n";
        }
    } else {
        cout << "\n-> Operacion cancelada\n";
    }
}
// Pantalla que muestra los perfiles y espera confirmación para volver
static void pantallaListarPerfiles(ListaPerfiles& listaPerfiles, const string& rutaPerfiles) {
    listarPerfiles(listaPerfiles, rutaPerfiles);
    // Espera a que el usuario presione 1 para volver al menu anterior
    int op = -1;
    while (op != 1) {
        cout << "\n1) para Volver\n";
        op = leerOpcion();
    }
}

static void pantallaEliminarPerfil(ListaPerfiles& listaPerfiles, const string& rutaPerfiles) {
    cout << "\n------------------------------------------------------------\n";
    cout << "Eliminar Perfiles\n";
    cout << "------------------------------------------------------------\n";

    if (!listaPerfiles.cargadoEnMemoria) {
        cargarPerfilesDesdeArchivo(listaPerfiles, rutaPerfiles);
    }

    string nombreBorrar = leerLinea("Nombre del perfil a borrar: ");
    
    if (!existePerfil(listaPerfiles, nombreBorrar)) {
        cout << "Error! No existe ningun perfil denominado '" << nombreBorrar << "'.\n";
        return;
    }

    // Alerta por si vamos a eliminar a un admin 
    if (nombreBorrar == "ADMIN") {
        cout << "\n============================================================\n";
        cout << "ALERTA!: Esta a punto de eliminar el perfil 'ADMIN'.\n";
    }

    cout << "\n1) guardar    2) cancelar\n";
    int op = leerOpcion();
    if (op == 1) {
        if (eliminarPerfilPorNombre(listaPerfiles, nombreBorrar, rutaPerfiles)) {
            cout << "\n-> Perfil eliminado\n";
        } else {
            cout << "\n-> No se logro eliminar el perfil\n";
        }
    } else {
        cout << "\n-> Operacion cancelada\n";
    }
}

void menuGestionPerfiles(ListaPerfiles& listaPerfiles, const string& rutaPerfiles) {
    int opcion = -1;
    while (opcion != 0){
        cout << "\n------------------------------------------------------------\n";
        cout << "GESTION DE PERFILES\n";
        cout << "------------------------------------------------------------\n";
        cout << "0) Salir\n";
        cout << "1) Ingresar un perfil\n";
        cout << "2) Listar los perfiles\n";
        cout << "3) Eliminar un perfil\n";
        
        opcion = leerOpcion();

        switch (opcion) {
            case 0:
                cout << "Volviendo al menu principal...\n";
                break;
            case 1:
                pantallaIngresarPerfil(listaPerfiles, rutaPerfiles);
                break;
            case 2:
                pantallaListarPerfiles(listaPerfiles, rutaPerfiles);
                break;
            case 3:
                pantallaEliminarPerfil(listaPerfiles, rutaPerfiles);
                break;
            default:
                cout << "Opcion invalida, intenta nuevamente\n";
                break;
        }
    }
}

// MENU PRINCIPAL

void mostrarMenuPrincipal(ListaUsuarios& listaUsuarios, ListaPerfiles& listaPerfiles, const string& rutaUsuarios, const string& rutaPerfiles) {
    int opcion = -1;
    while (opcion != 0) {
        cout << "------------------------------------------------------------\n";
        cout << "          Modulo: Administrador de Usuarios y Perfiles      \n";
        cout << "------------------------------------------------------------\n";
        cout << "1) Modulo - Gestion de Usuarios\n";
        cout << "2) Modulo - Gestion de Perfiles\n";
        cout << "0) Salir\n";

        opcion = leerOpcion();

        switch (opcion) {
            case 0:
                break;
            case 1:
                menuGestionUsuarios(listaUsuarios, listaPerfiles, rutaUsuarios, rutaPerfiles);
                break;
            case 2:
                menuGestionPerfiles(listaPerfiles, rutaPerfiles);
                break;
            default:
                cout << "Opcion invalida, por favor seleccione un numero del menu\n";
                break;
        }
    }
}


