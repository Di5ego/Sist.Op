#include "../include/menu.h"
#include "../include/conteo.h"
#include "../include/utilidades.h"
#include "../include/matriz.h"
#include "../include/env.h"

#include <iostream>
#include <limits>
#include <sstream>
#include <algorithm>
#include <cstdlib>

using namespace std;

// Función para leer y validar el ingreso de una opción numérica por consola.
static int leerOpcion() {
    // Declara la variable donde se guardará el número ingresado.
    int opcion = 0;  
    // Muestra el prompt solicitando la opción al usuario.
    cout << "\nOpcion : ";
    // Inicia un ciclo que se repite si la lectura falla (ej. si se ingresa un texto en vez de número).
    while (!(cin >> opcion)) {
        // Si se detecta el fin de flujo (EOF), retorna 0 por seguridad para evitar cuelgues.
        if (cin.eof()) return 0;
        // Limpia los indicadores de error del flujo de entrada.
        cin.clear();
        // Ignora el resto de la línea en el buffer para evitar un bucle infinito.
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        // Muestra un mensaje pidiendo un número válido.
        cout << "Entrada invalida. Ingrese un numero para la opcion: ";
    }
    // Una vez leído correctamente, limpia el salto de línea residual en el buffer.
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    // Retorna la opción ingresada.
    return opcion;
}

// Función para mostrar un mensaje (prompt) y leer una línea de texto completa.
static string leerLinea(const string& prompt) {
    // Declara una variable string para almacenar la entrada.
    string input;
    // Muestra el mensaje recibido como parámetro en consola.
    cout << prompt;
    // Utiliza getline para leer toda la línea; si falla, retorna una cadena vacía.
    if (!getline(cin, input)) {
        return "";
    }
    // Si tiene éxito, retorna el texto leído.
    return input;
}

// PANTALLAS DE GESTION DE USUARIOS (ENTREGA 1)

// Interfaz interactiva para el ingreso y validación de un nuevo usuario en el sistema.
static void pantallaIngresarUsuario(ListaUsuarios& listaUsuarios, ListaPerfiles& listaPerfiles, const string& rutaUsuarios, const string& rutaPerfiles) {
    // Imprime el título de la pantalla.
    cout << "Ingreso de usuarios\n";
    
    // Verifica si la lista de usuarios está en memoria; si no, la carga desde el archivo.
    if (!listaUsuarios.cargadoEnMemoria) {
        cargarUsuariosDesdeArchivo(listaUsuarios, rutaUsuarios);
    }
    // Verifica si la lista de perfiles está en memoria; si no, la carga desde el archivo.
    if (!listaPerfiles.cargadoEnMemoria) {
        cargarPerfilesDesdeArchivo(listaPerfiles, rutaPerfiles);
    }

    // Instancia un objeto temporal para almacenar los datos del nuevo usuario.
    Usuario u;
    
    // Ciclo para solicitar y validar el ID del usuario.
    while (true) {
        // Pide el ID y lo lee como texto.
        string idStr = leerLinea("Id: ");
        // Verifica que la entrada no esté vacía.
        if (idStr.empty()) {
            cout << "Error! El ID no puede estar vacio\n";
            continue;
        }
        try {
            // Intenta convertir el texto a entero.
            u.id = stoi(idStr);
            // Verifica que el ID sea mayor a cero.
            if (u.id <= 0) {
                cout << "Error! El ID debe ser un numero entero positivo mayor a 0\n";
                continue;
            }
            // Verifica si ya existe un usuario con ese mismo ID.
            if (existeUsuarioId(listaUsuarios, u.id)) {
                cout << "Error! Ya existe un usuario registrado con el ID " << u.id << ". Intente con otro\n";
                continue;
            }
            // Si pasa todas las validaciones, sale del ciclo.
            break;
        } catch (...) {
            // Captura errores de conversión si se ingresaron letras o caracteres inválidos.
            cout << "Error! El ID debe ser un numero entero valido\n";
        }
    }

    // Ciclo para solicitar y validar el Nombre del usuario.
    while (true) {
        // Lee el nombre ingresado.
        u.nombre = leerLinea("Nombre (Primer nombre y primer apellido): ");
        // Valida que el nombre no esté vacío.
        if (u.nombre.empty()) {
            cout << "Error! El nombre no puede estar vacio\n";
            continue;
        }
        // Valida que tenga al menos 5 caracteres.
        if (u.nombre.length() < 5) {
            cout << "Error! El nombre debe tener un minimo de 5 caracteres.\n";
            continue;
        }
        break;
    }

    // Ciclo para solicitar y validar el nombre de usuario (Username).
    while (true) {
        u.username = leerLinea("username: ");
        // Valida que no esté vacío.
        if (u.username.empty()) {
            cout << "Error! El username no puede estar vacio\n";
            continue;
        }
        // Valida que tenga al menos 2 caracteres.
        if (u.username.length() < 2) {
            cout << "Error! El username debe tener un minimo de 2 caracteres\n";
            continue;
        }
        // Verifica que el username no esté en uso por otro usuario.
        if (existeUsername(listaUsuarios, u.username)) {
            cout << "Error! El username '" << u.username << "' ya existe. Por favor elija uno diferente\n";
            continue;
        }
        break;
    }

    // Ciclo para solicitar y validar la contraseña.
    while (true) {
        // Lee la contraseña.
        u.password = leerLinea("password: ");
        // Valida que no esté vacía.
        if (u.password.empty()) {
            cout << "Error! La contraseña no puede estar vacia\n";
            continue;
        }
        // Valida que tenga un mínimo de 2 caracteres.
        if (u.password.length() < 2) {
            cout << "Error! La contraseña debe tener un minimo de 2 caracteres\n";
            continue;
        }
        // Sale del ciclo si es válida.
        break;
    }
    
    // Ciclo para solicitar y asignar el Perfil del usuario.
    while (true) {
        // Lee el nombre del perfil solicitado.
        u.perfil = leerLinea("perfil: ");
        // Valida que no esté vacío.
        if (u.perfil.empty()) {
            cout << "Error! El perfil no puede estar vacio\n";
            continue;
        }
        // Valida longitud mínima.
        if (u.perfil.length() < 2) {
            cout << "Error! El nombre del perfil debe tener un minimo de 2 caracteres\n";
            continue;
        }
        // Verifica que el perfil exista en el sistema antes de asignarlo.
        if (!existePerfil(listaPerfiles, u.perfil)) {
            cout << "Error! El perfil '" << u.perfil << "' no se encuentra registrado en los perfiles del sistema.\n";
            continue;
        } else {
            break;
        }
    }

    // Solicita confirmación para guardar al usuario.
    cout << "\n1) guardar    2) cancelar\n";
    // Lee la decisión.
    int op = leerOpcion();
    if (op == 1) {
        // Si acepta, intenta persistir el usuario en el archivo.
        if (guardarUsuario(listaUsuarios, u, rutaUsuarios)) {
            // Muestra mensaje de éxito.
            cout << "\n-> Usuario guardado exitosamente.\n";
        } else {
            // Muestra mensaje si ocurrió un error al escribir el archivo.
            cout << "\n-> Error al guardar el usuario.\n";
        }
    } else {
        // Si cancela, descarta los datos y vuelve al menú.
        cout << "\n-> Operacion cancelada\n";
    }
}

// Pantalla que muestra el listado de todos los usuarios registrados.
static void pantallaListarUsuarios(ListaUsuarios& listaUsuarios, const string& rutaUsuarios) {
    // Llama a la función que imprime en consola el registro de usuarios.
    listarUsuarios(listaUsuarios, rutaUsuarios);
    
    // Inicializa la opción de control del bucle.
    int op = -1;
    // Mantiene la pantalla visible hasta que se ingrese 1.
    while (op != 1) {
        // Muestra la indicación para salir.
        cout << "\n1) para Volver\n";
        // Lee la opción.
        op = leerOpcion();
    }
}

// Interfaz para buscar y eliminar un usuario existente mediante su ID.
static void pantallaEliminarUsuario(ListaUsuarios& listaUsuarios, const string& rutaUsuarios) {
    // Imprime el encabezado visual.
    cout << "\n------------------------------------------------------------\n";
    cout << "Eliminar Usuarios\n";
    cout << "------------------------------------------------------------\n";

    // Verifica que la lista esté cargada en memoria.
    if (!listaUsuarios.cargadoEnMemoria) {
        cargarUsuariosDesdeArchivo(listaUsuarios, rutaUsuarios);
    }

    // Solicita al usuario ingresar el ID del usuario que desea eliminar.
    string idStr = leerLinea("ID usuario a borrar: ");
    int idBorrar = -1;
    try {
        // Convierte la entrada de texto a entero.
        idBorrar = stoi(idStr);
    } catch (...) {
        // Detiene la función si el texto ingresado no era un número.
        cout << "Error! ID invalido.\n";
        return;
    }

    // Busca el usuario por ID utilizando find_if y una función lambda.
    auto it = find_if(listaUsuarios.usuarios.begin(), listaUsuarios.usuarios.end(), [idBorrar](const Usuario& u) {
        return u.id == idBorrar;
    });

    // Si no se encuentra ninguna coincidencia, notifica y retorna.
    if (it == listaUsuarios.usuarios.end()) {
        cout << "Error! No existe ningun usuario registrado con el ID " << idBorrar << ".\n";
        return;
    }

    // Alerta de seguridad si se está intentando eliminar a un administrador.
    if (it->perfil == "ADMIN") {
        cout << "ALERTA!: El usuario que intenta eliminar tiene perfil 'ADMIN'\n";
        cout << "Eliminar un usuario administrador puede provocar perdida de acceso\n";
    }

    // Solicita confirmación de la eliminación.
    cout << "\n1) guardar    2) cancelar\n";
    // Lee la decisión del usuario.
    int op = leerOpcion();
    if (op == 1) {
        // Si acepta, procede con la eliminación e informa el resultado.
        if (eliminarUsuarioPorId(listaUsuarios, idBorrar, rutaUsuarios, true)) {
            cout << "\n-> Usuario eliminado exitosamente\n";
        } else {
            cout << "\n-> No se pudo eliminar el usuario\n";
        }
    } else {
        // Si cancela, no se realizan cambios.
        cout << "\n-> Operacion cancelada\n";
    }
}

// Muestra un menú interactivo en la consola para gestionar usuarios. 
void menuGestionUsuarios(ListaUsuarios& listaUsuarios, ListaPerfiles& listaPerfiles, const string& rutaUsuarios, const string& rutaPerfiles) {
    // Inicializa la variable de control del bucle.
    int opcion = -1;
    // Mantiene al usuario en un ciclo donde puede elegir opciones hasta que ingrese 0.
    while (opcion != 0) {
        // Muestra las opciones disponibles.
        cout << "\nModulo - Gestion de Usuarios\n";
        cout << "0) Salir / Volver\n";
        cout << "1) Ingresar Usuarios\n";
        cout << "2) Listar Usuarios\n";
        cout << "3) Eliminar Usuarios\n";
        
        // Solicita y lee la opción elegida.
        opcion = leerOpcion();

        // Evalúa la opción ingresada para dirigir el flujo.
        switch (opcion) {
            case 0:
                // Rompe el switch y termina el bucle.
                break;
            case 1:
                // Llama a la pantalla para crear usuario.
                pantallaIngresarUsuario(listaUsuarios, listaPerfiles, rutaUsuarios, rutaPerfiles);
                break;
            case 2:
                // Llama a la pantalla para ver usuarios.
                pantallaListarUsuarios(listaUsuarios, rutaUsuarios);
                break;
            case 3:
                // Llama a la pantalla para borrar un usuario.
                pantallaEliminarUsuario(listaUsuarios, rutaUsuarios);
                break;
            default:
                // Advierte sobre ingreso incorrecto y repite el menú.
                cout << "Opcion invalida, intente de nuevo\n";
                break;
        }
    }
}

// PANTALLAS DE GESTION DE PERFILES (ENTREGA 1)

// Interfaz interactiva para registrar un nuevo perfil y asignar sus permisos de menú.
static void pantallaIngresarPerfil(ListaPerfiles& listaPerfiles, const string& rutaPerfiles) {
    // Imprime el título.
    cout << "Ingreso de perfiles\n";

    // Si los perfiles no están en memoria, los carga desde el archivo.
    if (!listaPerfiles.cargadoEnMemoria) {
        cargarPerfilesDesdeArchivo(listaPerfiles, rutaPerfiles);
    }

    // Instancia un objeto Perfil temporal.
    Perfil p;
    // Ciclo para solicitar y validar el nombre del perfil.
    while (true) {
        // Lee el nombre ingresado por teclado.
        p.nombre = leerLinea("Nombre del perfil (ej: ADMIN, GENERAL): ");
        // Valida que no esté vacío.
        if (p.nombre.empty()) {
            cout << "Error! El nombre del perfil no puede estar vacio\n";
            continue;
        }
        // Valida que tenga al menos 2 caracteres.
        if (p.nombre.length() < 2) {
            cout << "Error! El nombre del perfil debe tener un minimo de 2 caracteres\n";
            continue;
        }
        // Verifica que no exista un perfil con ese mismo nombre.
        if (existePerfil(listaPerfiles, p.nombre)) {
            cout << "Error! Ya existe un perfil denominado '" << p.nombre << "'. Por favor elija un nombre diferente\n";
            continue;
        }
        // Sale del bucle si es correcto.
        break;
    }

    // Ciclo para solicitar y validar las opciones numéricas del perfil.
    while (true) {
        // Solicita los permisos de menú en formato de texto separado por comas.
        string opsLine = leerLinea("Opciones de menu permitidas (separadas por coma, ej: 0,1,2,3): ");
        // Valida que no esté vacío.
        if (opsLine.empty()) {
            cout << "Error! Debe ingresar al menos una opcion de menu permitida\n";
            continue;
        }

        // Variable para controlar si todos los caracteres son válidos.
        bool caracteresValidos = true;
        // Recorre la cadena validando que sólo contenga números y comas.
        for (char c : opsLine) {
            if (!isdigit(static_cast<unsigned char>(c)) && c != ',') {
                caracteresValidos = false;
                break;
            }
        }

        // Si se encontraron caracteres inválidos, avisa al usuario y repite.
        if (!caracteresValidos) {
            cout << "Error! Solamente se permiten numeros enteros positivos y comas (',') como separador\n";
            continue;
        }

        // Limpia cualquier opción que se haya guardado en intentos fallidos previos.
        p.opciones.clear();
        // Usa stringstream para separar la entrada por comas.
        stringstream ss(opsLine);
        string token;
        bool formatoValido = true;

        // Bucle que extrae cada número separado por coma.
        while (getline(ss, token, ',')) {
            // Detecta comas seguidas o vacías (ej: "1,,2").
            if (token.empty()) {
                formatoValido = false;
                break;
            }
            try {
                // Convierte el string del número a entero.
                int num = stoi(token);
                // Valida que no existan números negativos.
                if (num < 0) {
                    formatoValido = false;
                    break;
                }
                // Añade la opción válida al vector de opciones del perfil.
                p.opciones.push_back(num);
            } catch (...) {
                // Captura cualquier error residual de conversión.
                formatoValido = false;
                break;
            }
        }

        // Verifica que la cadena no termine en una coma colgante (ej: "1,2,").
        if (!opsLine.empty() && opsLine.back() == ',') {
            formatoValido = false;
        }

        // Si hubo algún fallo estructural o la lista de opciones quedó vacía, informa y repite.
        if (!formatoValido || p.opciones.empty()) {
            cout << "Error! Formato invalido de opciones. Ingrese solo numeros enteros positivos separados por coma (ej: 0,1,2)\n";
            p.opciones.clear();
            continue;
        }
        // Sale del ciclo si todas las opciones fueron parseadas correctamente.
        break;
    }

    // Solicita confirmación para registrar el perfil.
    cout << "\n1) guardar    2) cancelar\n";
    // Lee la decisión final.
    int op = leerOpcion();
    if (op == 1) {
        // Intenta guardar el perfil y notifica el resultado.
        if (guardarPerfil(listaPerfiles, p, rutaPerfiles)) {
            cout << "\n-> Perfil guardado exitosamente\n";
        } else {
            cout << "\n-> Error al guardar el perfil\n";
        }
    } else {
        // Avisa que la operación fue abortada.
        cout << "\n-> Operacion cancelada\n";
    }
}

// Pantalla que muestra el listado de todos los perfiles registrados.
static void pantallaListarPerfiles(ListaPerfiles& listaPerfiles, const string& rutaPerfiles) {
    // Delega la impresión del registro a la función listarPerfiles.
    listarPerfiles(listaPerfiles, rutaPerfiles);
    // Variable de control del ciclo.
    int op = -1;
    // Mantiene la pantalla activa hasta pulsar 1.
    while (op != 1) {
        // Muestra el mensaje para volver.
        cout << "\n1) para Volver\n";
        // Lee la opción.
        op = leerOpcion();
    }
}

// Interfaz para eliminar un perfil y, en cascada, a los usuarios que posean ese perfil.
static void pantallaEliminarPerfil(ListaPerfiles& listaPerfiles, ListaUsuarios& listaUsuarios, const string& rutaPerfiles, const string& rutaUsuarios) {
    // Imprime el encabezado.
    cout << "\n------------------------------------------------------------\n";
    cout << "Eliminar Perfiles\n";
    cout << "------------------------------------------------------------\n";

    // Carga los perfiles en memoria si no están disponibles.
    if (!listaPerfiles.cargadoEnMemoria) {
        cargarPerfilesDesdeArchivo(listaPerfiles, rutaPerfiles);
    }

    // Pide al usuario escribir el nombre exacto del perfil a eliminar.
    string nombreBorrar = leerLinea("Nombre del perfil a borrar: ");
    
    // Valida que el perfil ingresado realmente exista.
    if (!existePerfil(listaPerfiles, nombreBorrar)) {
        cout << "Error! No existe ningun perfil denominado '" << nombreBorrar << "'.\n";
        return;
    }

    // Muestra una alerta especial si se intenta borrar el perfil de administrador.
    if (nombreBorrar == "ADMIN") {
        cout << "\n============================================================\n";
        cout << "ALERTA!: Esta a punto de eliminar el perfil 'ADMIN'.\n";
    }

    // Solicita la decisión de continuar o cancelar.
    cout << "\n1) guardar    2) cancelar\n";
    // Recoge la opción elegida.
    int op = leerOpcion();
    if (op == 1) {
        // Si confirma, intenta borrar el perfil especificado.
        if (eliminarPerfilPorNombre(listaPerfiles, nombreBorrar, rutaPerfiles)) {
            cout << "\n-> Perfil '" << nombreBorrar << "' eliminado exitosamente\n";
            // Ejecuta el borrado en cascada de los usuarios asignados a ese perfil.
            int usuariosEliminados = eliminarUsuariosPorPerfil(listaUsuarios, nombreBorrar, rutaUsuarios);
            // Si se borraron usuarios, informa de la cantidad eliminada.
            if (usuariosEliminados > 0) {
                cout << "-> Se eliminaron en cascada " << usuariosEliminados << " usuario(s) asociados al perfil '" << nombreBorrar << "'.\n";
            }
        } else {
            // Muestra mensaje si ocurrió un error escribiendo los datos.
            cout << "\n-> No se logro eliminar el perfil\n";
        }
    } else {
        // Notifica la cancelación sin hacer cambios.
        cout << "\n-> Operacion cancelada\n";
    }
}

// Menú interactivo dedicado a la gestión completa de los perfiles.
void menuGestionPerfiles(ListaPerfiles& listaPerfiles, ListaUsuarios& listaUsuarios, const string& rutaPerfiles, const string& rutaUsuarios) {
    // Inicializa la variable de opción.
    int opcion = -1;
    // Bucle del menú que corre hasta que se escoja la opción de salida (0).
    while (opcion != 0){
        // Muestra el panel de opciones de perfiles.
        cout << "\n------------------------------------------------------------\n";
        cout << "GESTION DE PERFILES\n";
        cout << "------------------------------------------------------------\n";
        cout << "0) Salir / Volver\n";
        cout << "1) Ingresar un perfil\n";
        cout << "2) Listar los perfiles\n";
        cout << "3) Eliminar un perfil\n";
        
        // Lee la opción por consola.
        opcion = leerOpcion();

        // Enruta hacia la funcionalidad solicitada.
        switch (opcion) {
            case 0:
                // Finaliza el menú.
                cout << "Volviendo...\n";
                break;
            case 1:
                // Llama al creador de perfiles.
                pantallaIngresarPerfil(listaPerfiles, rutaPerfiles);
                break;
            case 2:
                // Llama al listado de perfiles.
                pantallaListarPerfiles(listaPerfiles, rutaPerfiles);
                break;
            case 3:
                // Llama a la interfaz de eliminación de perfiles.
                pantallaEliminarPerfil(listaPerfiles, listaUsuarios, rutaPerfiles, rutaUsuarios);
                break;
            default:
                // Maneja valores erróneos de menú.
                cout << "Opcion invalida, intenta nuevamente\n";
                break;
        }
    }
}

// Submenú de la Opcion 1: Permite acceder al sistema de Administracion
// de Usuarios y Perfiles construido en la Entrega 1 (restringido a perfil ADMIN).
static void pantallaAdminUsuariosYPerfiles(ListaUsuarios& listaUsuarios, ListaPerfiles& listaPerfiles, const string& rutaUsuarios, const string& rutaPerfiles) {
    // Variable para controlar el bucle de este submenú.
    int op = -1;
    // Mantiene el submenú activo hasta elegir volver (0).
    while (op != 0) {
        // Muestra el encabezado y las rutas a los módulos de gestión.
        cout << "\n============================================================\n";
        cout << "    ADMINISTRACION DE USUARIOS Y PERFILES (Llamada Entrega 1) \n";
        cout << "============================================================\n";
        cout << "1) Modulo - Gestion de Usuarios\n";
        cout << "2) Modulo - Gestion de Perfiles\n";
        cout << "0) Volver al Menú Principal\n";

        // Captura la elección.
        op = leerOpcion();

        // Deriva al módulo correspondiente.
        switch (op) {
            case 0:
                // Sale del switch para retornar.
                break;
            case 1:
                // Accede al gestor de usuarios.
                menuGestionUsuarios(listaUsuarios, listaPerfiles, rutaUsuarios, rutaPerfiles);
                break;
            case 2:
                // Accede al gestor de perfiles.
                menuGestionPerfiles(listaPerfiles, listaUsuarios, rutaPerfiles, rutaUsuarios);
                break;
            default:
                // Controla entradas inválidas.
                cout << "Opcion invalida.\n";
                break;
        }
    }
}

// Función utilitaria para limpiar espacios en blanco y comillas alrededor de una cadena,
// especialmente útil al leer rutas de archivos arrastradas/pegadas en la consola.
static string limpiarComillas(const string& str) {
    // Busca el índice del primer carácter que no sea espacio, salto de línea o comilla.
    size_t first = str.find_first_not_of(" \t\r\n\"'");
    // Si no se encuentra ninguno (cadena vacía o solo caracteres a ignorar), retorna cadena vacía.
    if (first == string::npos) return "";
    // Busca el índice del último carácter válido con la misma exclusión.
    size_t last = str.find_last_not_of(" \t\r\n\"'");
    // Retorna la subcadena que abarca desde el primer hasta el último carácter válido.
    return str.substr(first, (last - first + 1));
}

// Interfaz para el programa de multiplicación de matrices NxM.
static void pantallaMultiplicaMatrices(const SesionUsuario& sesion) {
    // Muestra el encabezado del programa.
    cout << "\n============================================================\n";
    cout << "           MULTIPLICACION DE MATRICES NxM (Programa 2)      \n";
    cout << "============================================================\n";
    
    // Solicita la ruta del archivo A y la limpia de comillas o espacios.
    string rutaA = limpiarComillas(leerLinea("Ingrese la ruta del archivo A.TXT: "));
    // Valida que no esté vacía.
    if (rutaA.empty()) {
        cout << "Operacion cancelada (Ruta A vacia).\n";
        return;
    }
    
    // Solicita la ruta del archivo B y la limpia.
    string rutaB = limpiarComillas(leerLinea("Ingrese la ruta del archivo B.TXT: "));
    // Valida que no esté vacía.
    if (rutaB.empty()) {
        cout << "Operacion cancelada (Ruta B vacia).\n";
        return;
    }
    
    // Solicita el separador de elementos (ej. # o ,).
    string sepStr = leerLinea("Ingrese el separador de elementos (ej: #, ',', esp en blanco): ");
    // Si se dejó vacío, asume espacio por defecto.
    if (sepStr.empty()) {
        sepStr = " ";
    } else {
        // Limpia el separador introducido y verifica nuevamente.
        sepStr = limpiarComillas(sepStr);
        if (sepStr.empty()) sepStr = " ";
    }

    // Toma únicamente el primer carácter como separador.
    char separador = sepStr[0];

    // Imprime el inicio de la ejecución formal con datos del usuario logueado.
    cout << "\n============================================================\n";
    cout << "          PROGRAMA MULTIPLICADOR DE MATRICES (NxM)           \n";
    cout << "User: " << sesion.usuario.username << " (" << sesion.perfil.nombre << ")\n";
    cout << "============================================================\n\n";

    // Intenta cargar la matriz A desde el archivo especificado.
    cout << "Cargando Matriz A desde: " << rutaA << " (Separador: '" << separador << "')\n";
    ResultadoMatriz resA = cargarMatriz(rutaA, separador);
    // Si falla la carga, muestra el error y finaliza.
    if (!resA.exito) {
        cout << "Error en Matriz A: " << resA.error << "\n";
        return;
    }
    // Si tiene éxito, informa las dimensiones e imprime la matriz cargada.
    cout << "Matriz A cargada correctamente (" << resA.filas << "x" << resA.columnas << "):\n";
    imprimirMatriz(resA.datos);

    // Intenta cargar la matriz B desde su respectivo archivo.
    cout << "\nCargando Matriz B desde: " << rutaB << " (Separador: '" << separador << "')\n";
    ResultadoMatriz resB = cargarMatriz(rutaB, separador);
    // Si falla, informa y cancela.
    if (!resB.exito) {
        cout << "Error en Matriz B: " << resB.error << "\n";
        return;
    }
    // Si tiene éxito, informa sus dimensiones y la imprime.
    cout << "Matriz B cargada correctamente (" << resB.filas << "x" << resB.columnas << "):\n";
    imprimirMatriz(resB.datos);

    // Notifica que iniciará el algoritmo de multiplicación.
    cout << "\nCalculando producto A x B...\n";
    // Calcula la matriz resultante.
    ResultadoMatriz resC = multiplicarMatrices(resA.datos, resB.datos);
    // Si la multiplicación es inválida algebraicamente, muestra el error.
    if (!resC.exito) {
        cout << "\nError en la multiplicacion: " << resC.error << "\n";
        return;
    }

    // Muestra visualmente la matriz C final obtenida del cálculo.
    cout << "\n============================================================\n";
    cout << "MATRIZ RESULTANTE C = A x B (" << resC.filas << "x" << resC.columnas << "):\n";
    cout << "============================================================\n";
    imprimirMatriz(resC.datos);
    cout << "============================================================\n";
}

// Menú principal unificado de la aplicación (Entrega 2) con control de acceso por sesión.
void mostrarMenuPrincipalEntrega2(SesionUsuario& sesion, ListaUsuarios& listaUsuarios, ListaPerfiles& listaPerfiles, const string& rutaUsuarios, const string& rutaPerfiles) {
    // Variable de control del ciclo principal.
    int opcion = -1;
    // Bucle maestro que sostiene la aplicación activa hasta recibir un 0 (Salir).
    while (opcion != 0) {
        // Despliega constantemente los datos del usuario logueado en la sesión.
        cout << "\n============================================================\n";
        cout << "                       MENU PRINCIPAL                       \n";
        cout << "User: " << sesion.usuario.username << " (" << sesion.perfil.nombre << ")\n";
        cout << "============================================================\n";
        
        // Muestra todas las opciones de los programas requeridos.
        cout << "0) Salir\n";
        cout << "1) Admin de usuarios y perfiles\n";
        cout << "2) Multiplica matrices NxM\n";
        cout << "3) Juego\n";
        cout << "4) Es palindromo?\n";
        cout << "5) Calcular f(x)=x*x + 2x + 8\n";
        cout << "6) CONTEO SOBRE TEXTO\n";
        cout << "7) CONTEO SOBRE ARCHIVO\n";
        cout << "============================================================\n";

        // Captura la elección introducida.
        opcion = leerOpcion();

        // Enruta hacia el módulo correspondiente.
        switch (opcion) {
            case 0:
                // Mensaje de despedida antes de finalizar el programa.
                cout << "Saliendo del sistema...\n";
                break;
            case 1:
                // Bloquea el acceso si el usuario no tiene el perfil 'ADMIN'.
                if (sesion.perfil.nombre != "ADMIN") {
                    cout << "\n============================================================\n";
                    cout << "ACCESO DENEGADO: Su perfil '" << sesion.perfil.nombre << "' no posee permisos de ADMINISTRACION.\n";
                    cout << "Solamente usuarios con perfil ADMIN pueden ingresar a la opcion (1).\n";
                    cout << "============================================================\n";
                } else {
                    // Si es administrador, abre el submenú de gestión.
                    pantallaAdminUsuariosYPerfiles(listaUsuarios, listaPerfiles, rutaUsuarios, rutaPerfiles);
                }
                break;
            case 2:
                // Llama al algoritmo visual de multiplicación de matrices.
                pantallaMultiplicaMatrices(sesion);
                break;
            case 3:
                // Placeholder para la opción del Juego.
                cout << "\n============================================================\n";
                cout << "(mensaje en construccion)\n";
                cout << "============================================================\n";
                break;
            case 4:
                // Inicia el evaluador de palíndromos.
                menuPalindromo();
                break;
            case 5:
                // Inicia la calculadora de la función cuadrática.
                menuCalcularFuncion();
                break;
            case 6:
                // Ejecuta la función de conteo usando el archivo predeterminado capturado con la bandera -f.
                mostrarInterfazConteo(sesion.archivoPorDefecto, "CONTEO SOBRE TEXTO (Archivo -f)");
                break;
            case 7: {
                // Pide interactivamente la ruta de cualquier archivo para su evaluación.
                string rutaPersonalizada = limpiarComillas(leerLinea("Ingrese la ruta del archivo a analizar: "));
                mostrarInterfazConteo(rutaPersonalizada, "CONTEO SOBRE ARCHIVO");
                break;
            }
            default:
                // Detecta e informa sobre entradas fuera del rango 0-7.
                cout << "Opcion invalida. Por favor seleccione una opcion valida (0-7).\n";
                break;
        }
    }
}