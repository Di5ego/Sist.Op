#include "../include/menu.h"
#include "../include/conteo.h"
#include "../include/utilidades.h"
#include "../include/env.h"
#include "../include/externo.h"            // llamadas a sistema a los programas de cada entrega
#include "../include/juego.h"              // Entrega 3 - Problema 2 (submenu del juego)

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
// Invoca al programa independiente multi.exe como programa externo, tal como exige la pauta oficial.
static void pantallaMultiplicaMatrices(const SesionUsuario& sesion) {
    // Muestra el encabezado del programa.
    cout << "\n============================================================\n";
    cout << "           MULTIPLICACION DE MATRICES NxM (Programa 2)      \n";
    cout << "============================================================\n";
    
    // Solicita la ruta del archivo A y la limpia de comillas o espacios.
    string rutaA = limpiarComillas(leerLinea("Ingrese la ruta del archivo A.TXT: "));
    if (rutaA.empty()) {
        cout << "Operacion cancelada (Ruta A vacia).\n";
        return;
    }
    
    // Solicita la ruta del archivo B y la limpia.
    string rutaB = limpiarComillas(leerLinea("Ingrese la ruta del archivo B.TXT: "));
    if (rutaB.empty()) {
        cout << "Operacion cancelada (Ruta B vacia).\n";
        return;
    }
    
    // Solicita el separador de elementos (ej. # o ,).
    string sepStr = leerLinea("Ingrese el separador de elementos (ej: #, ',', esp en blanco): ");
    if (sepStr.empty()) {
        sepStr = " ";
    } else {
        sepStr = limpiarComillas(sepStr);
        if (sepStr.empty()) sepStr = " ";
    }

    if (sepStr.size() != 1) {
        cout << "Error! El separador debe ser un solo caracter.\n";
        return;
    }

    // Llamada a sistema al programa multiplicador (Entrega 2), con las rutas,
    // el separador, el usuario y el perfil como argumentos.
    llamarPrograma("MATRIX_EXEC", "Entrega 2 - Multiplicador de matrices",
                   {rutaA, rutaB, sepStr, string(sesion.usuario.username), string(sesion.perfil.nombre)});
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
        cout << "8) Crear indice invertido\n";
        cout << "============================================================\n";

        // Captura la elección introducida.
        opcion = leerOpcion();

        if (opcion < 0 || opcion > 8) {
            cout << "Opcion invalida. Por favor seleccione una opcion valida (0-8).\n";
            continue;
        }

        // Control de acceso dinámico según las opciones permitidas por el perfil
        if (!perfilPermiteOpcion(sesion.perfil, opcion)) {
            cout << "\n============================================================\n";
            cout << "ACCESO DENEGADO: Su perfil '" << sesion.perfil.nombre << "' no posee permisos para la opcion (" << opcion << ").\n";
            if (opcion == 1) {
                cout << "Solamente usuarios con perfil ADMIN pueden ingresar a la opcion (1).\n";
            }
            cout << "============================================================\n";
            continue;
        }

        // Enruta hacia el módulo correspondiente.
        switch (opcion) {
            case 0:
                // Mensaje de despedida antes de finalizar el programa.
                cout << "Saliendo del sistema...\n";
                break;
            case 1:
                // Si es administrador, abre el submenú de gestión.
                // Llamada a sistema al programa de la Entrega 1 (admin_usuarios.exe).
                llamarPrograma("ADMIN_EXEC", "Entrega 1 - Administracion de usuarios y perfiles",
                               {string(sesion.usuario.username), string(sesion.perfil.nombre)});
                // El programa externo pudo modificar los archivos: se recargan los datos.
                listaUsuarios.cargadoEnMemoria = false;
                listaPerfiles.cargadoEnMemoria = false;
                cargarUsuariosDesdeArchivo(listaUsuarios, rutaUsuarios);
                cargarPerfilesDesdeArchivo(listaPerfiles, rutaPerfiles);
                break;
            case 2:
                // Llama al programa externo de multiplicación de matrices.
                pantallaMultiplicaMatrices(sesion);
                break;
            case 3:
                // Entrega 3 - Problema 2: juego multiplayer mediante sockets.
                pantallaJuego(string(sesion.usuario.username));
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

// -------------------------------------------------------------------------------------------------------------------------------------------------

            case 8:
                //1.1)COMO LLEGA EL USUARIO AL PROGRAMA 
                // ENTREGA 3 / PROBLEMA 1: la opción 8 NO contiene la lógica del índice. Solo llama a indice_invertido.exe (llamada a sistema), 
                // igual que la opción 1 llama a admin_usuarios.exe. La lógica está en indice_invertido.cpp.  
                llamarPrograma("INDEX_EXEC", "Entrega 3 - Indice invertido concurrente",
                               {string(sesion.usuario.username), string(sesion.perfil.nombre)});
                break;
        }
    }
}