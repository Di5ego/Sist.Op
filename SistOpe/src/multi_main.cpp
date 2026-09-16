#include <iostream>
#include <string>
#include "../include/matriz.h"

using namespace std;

// Punto de entrada para el ejecutable independiente multi.exe.
// Recibe rutas de matrices A y B, separador de elementos, y credenciales del usuario.
int main(int argc, char* argv[]) {
    if (argc < 4) {
        cerr << "Error: Argumentos insuficientes para el multiplicador de matrices.\n";
        cerr << "Uso: ./multi <ruta_A.TXT> <ruta_B.TXT> <separador> [username] [perfil]\n";
        cerr << "Ejemplo: ./multi \"a.txt\" \"b.txt\" \"#\"\n";
        return 1;
    }

    string rutaA = argv[1]; // Ruta del archivo de la Matriz A
    string rutaB = argv[2]; // Ruta del archivo de la Matriz B
    string sepStr = argv[3]; // Separador de elementos en los archivos de texto
    char separador = (sepStr.empty()) ? ' ' : sepStr[0]; // Usar el primer carácter del separador o espacio por defecto

    string username = (argc >= 5) ? argv[4] : "invitado"; // Nombre de usuario opcional
    string perfil = (argc >= 6) ? argv[5] : "GENERAL"; // Perfil de usuario opcional

    // Mostrar encabezado con credenciales de usuario y perfil
    cout << "============================================================\n";
    cout << "          PROGRAMA MULTIPLICADOR DE MATRICES (NxM)           \n";
    cout << "User: " << username << " (" << perfil << ")\n";
    cout << "============================================================\n\n";

    // Cargar y validar Matriz A
    cout << "Cargando Matriz A desde: " << rutaA << " (Separador: '" << separador << "')\n";
    ResultadoMatriz resA = cargarMatriz(rutaA, separador);
    if (!resA.exito) {
        cerr << "Error en Matriz A: " << resA.error << "\n";
        return 1;
    }
    cout << "Matriz A cargada correctamente (" << resA.filas << "x" << resA.columnas << "):\n";
    imprimirMatriz(resA.datos);

    // Cargar y validar Matriz B
    cout << "\nCargando Matriz B desde: " << rutaB << " (Separador: '" << separador << "')\n";
    ResultadoMatriz resB = cargarMatriz(rutaB, separador);
    if (!resB.exito) {
        cerr << "Error en Matriz B: " << resB.error << "\n";
        return 1;
    }
    cout << "Matriz B cargada correctamente (" << resB.filas << "x" << resB.columnas << "):\n";
    imprimirMatriz(resB.datos);

    // Calcular el producto C = A x B
    cout << "\nCalculando producto A x B...\n";
    ResultadoMatriz resC = multiplicarMatrices(resA.datos, resB.datos);
    if (!resC.exito) {
        cerr << "\nError en la multiplicacion: " << resC.error << "\n";
        return 1;
    }

    // Mostrar Matriz resultante
    cout << "\n============================================================\n";
    cout << "MATRIZ RESULTANTE C = A x B (" << resC.filas << "x" << resC.columnas << "):\n";
    cout << "============================================================\n";
    imprimirMatriz(resC.datos);
    cout << "============================================================\n";

    return 0;
}
