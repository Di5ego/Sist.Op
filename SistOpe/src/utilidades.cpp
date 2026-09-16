#include "../include/utilidades.h"
#include <iostream>
#include <cctype>
#include <algorithm>
#include <limits>
#include <iomanip>

using namespace std;

// Funcion para verificar si un texto es un palindromo
bool esPalindromo(const string& texto) {
    string limpia = "";
    // Convertir a minusculas y eliminar caracteres no alfanumericos
    for (char c : texto) {
        if (isalnum(static_cast<unsigned char>(c))) {
            limpia += tolower(static_cast<unsigned char>(c));
        }
    }
    // Verificar si la cadena limpia es un palindromo
    if (limpia.empty()) return false;

    int i = 0;
    int j = static_cast<int>(limpia.length()) - 1;
    // Comparar caracteres desde ambos extremos hacia el centro
    while (i < j) {
        if (limpia[i] != limpia[j]) return false;
        i++;
        j--;
    }
    return true;
}

// Funcion para calcular f(x) = x^2 + 2x + 8
double calcularFuncion(double x) {
    return (x * x) + (2.0 * x) + 8.0;
}

// Menu para validar palindromos
void menuPalindromo() {
    int op = -1;
    // Bucle principal del menu
    while (op != 2) {
        cout << "\n------------------------------------------------------------\n";
        cout << "                 VALIDADOR DE PALINDROMOS                   \n";
        cout << "------------------------------------------------------------\n";
        cout << "1) Validar texto\n";
        cout << "2) Cancelar / Volver\n";
        cout << "Opcion: ";
        // Validar que la opcion ingresada sea un numero entero
        if (!(cin >> op)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        // Validar la opcion ingresada
        if (op == 1) {
            cout << "Ingrese el texto a evaluar: ";
            string texto;
            getline(cin, texto);
            // Validar si el texto ingresado no esta vacio
            if (texto.empty()) {
                cout << "Error: El texto no puede estar vacio.\n";
                continue;
            }
            // Validar si el texto es un palindromo
            if (esPalindromo(texto)) {
                cout << "\n-> ¡El texto INGRESADO SI ES UN PALINDROMO!\n";
            } else {
                cout << "\n-> El texto ingresado NO ES UN PALINDROMO.\n";
            }
        }
    }
}

// Menu para calcular la funcion f(x) = x^2 + 2x + 8
void menuCalcularFuncion() {
    int op = -1;
    while (op != 2) {
        cout << "\n------------------------------------------------------------\n";
        cout << "             CALCULO DE f(x) = x*x + 2x + 8                \n";
        cout << "------------------------------------------------------------\n";
        cout << "1) Ingresar valor X\n";
        cout << "2) VOLVER\n";
        cout << "Opcion: ";
        // Validar que la opcion ingresada sea un numero entero
        if (!(cin >> op)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        // Validar la opcion ingresada
        if (op == 1) {
            cout << "Ingrese el valor de X (ej: 2.5, -4, 0): ";
            double x;
            // Validar que el valor ingresado sea un numero real
            if (cin >> x) {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                double res = calcularFuncion(x);
                cout << fixed << setprecision(4);
                cout << "\n-> Especificacion del calculo con numeros reales:\n";
                cout << "   f(" << x << ") = (" << x << ")^2 + 2*(" << x << ") + 8 = " << res << "\n";
            // Mensaje de error si el valor ingresado no es un numero real
            } else {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Error: Ingrese un numero real valido.\n";
            }
        }
    }
}
