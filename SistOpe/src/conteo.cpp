#include "../include/conteo.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cctype>

using namespace std;

static bool esVocal(char c) { // Funcion para determinar si un caracter es una vocal (considerando solo las vocales básicas sin acentos)
    char lower = tolower(static_cast<unsigned char>(c)); // Convertir a minuscula para simplificar la comparación
    return (lower == 'a' || lower == 'e' || lower == 'i' || lower == 'o' || lower == 'u'); // Considerar solo las vocales básicas sin acentos
}

static bool esConsonante(char c) { // Función para determinar si un caracter es una consonante (considerando solo las letras del alfabeto)
    if (!isalpha(static_cast<unsigned char>(c))) return false; // Si no es una letra, no es consonante
    return !esVocal(c); // Si es una letra y no es vocal, entonces es consonante
}

ResumenConteo contarTextoArchivo(const string& rutaArchivo) {
    // Define una función que recibe la ruta del archivo y devuelve
    // un objeto ResumenConteo con los resultados del análisis.
    ResumenConteo r;
    // Crea la variable 'r', donde se guardarán los conteos
    // de vocales, consonantes, caracteres especiales y palabras.

    ifstream file(rutaArchivo);
    // Abre el archivo indicado en modo lectura.

    if (!file.is_open()) {
        // Comprueba si el archivo se abrió correctamente.
        r.exito = false;
        // Indica que la operación no fue exitosa.
        r.mensajeError = "No se pudo abrir el archivo especificado: " + rutaArchivo;
        // Guarda un mensaje indicando cuál fue el problema.
        return r;
        // Termina la función y devuelve el resultado con el error.
    }

    r.exito = true;
    // Indica que el archivo se abrió correctamente.

    string palabra;
    // Variable que se utilizará posteriormente para guardar cada palabra.

    char ch;
    // Variable que almacenará cada carácter leído del archivo.

    // ---------------------------------------------------------
    // CONTEO DE CARACTERES
    // ---------------------------------------------------------

    
    while (file.get(ch)) {
        // Lee el archivo carácter por carácter.
        // El ciclo continúa mientras existan caracteres para leer.

        unsigned char uch = static_cast<unsigned char>(ch);
        // Convierte el carácter a unsigned char para poder trabajar
        // correctamente con valores de caracteres especiales.
        if (isspace(uch)) {
            // Comprueba si el carácter es un espacio, salto de línea,
            // tabulación u otro espacio en blanco.
            continue;
            // Ignora ese carácter y pasa directamente al siguiente.
        }

        // ---------------------------------------------------------
        // MANEJO DE CARACTERES ESPAÑOLES EN UTF-8
        // ---------------------------------------------------------

        if (uch == 0xC3) {
            // Comprueba si comienza una secuencia UTF-8 utilizada
            // por caracteres españoles como á, é, í, ó, ú, ñ y ü.
            char nextCh;
            // Variable para almacenar el segundo byte del carácter.
            if (file.get(nextCh)) {
                // Lee el siguiente byte del archivo.

                unsigned char nextUch = static_cast<unsigned char>(nextCh);
                // Convierte el segundo byte a unsigned char.

                if (nextUch == 0xA1 || nextUch == 0x81 ||
                    // á, Á
                    nextUch == 0xA9 || nextUch == 0x89 ||
                    // é, É
                    nextUch == 0xAD || nextUch == 0x8D ||
                    // í, Í
                    nextUch == 0xB3 || nextUch == 0x93 ||
                    // ó, Ó
                    nextUch == 0xBA || nextUch == 0x9A ||
                    // ú, Ú
                    nextUch == 0xBC || nextUch == 0x9C) {
                    // ü, Ü
                    r.vocales++;
                    // Si corresponde a una vocal con tilde o ü,
                    // aumenta el contador de vocales.
                }

                else if (nextUch == 0xB1 || nextUch == 0x91) {
                    // Comprueba si el carácter corresponde a ñ o Ñ.
                    r.consonantes++;
                    // La ñ es una consonante, por lo que aumenta
                    // el contador de consonantes.
                }
                else {
                    r.especiales++;
                    // Si no corresponde a una vocal ni a ñ,
                    // se considera un carácter especial.
                }
                continue;
                // Termina esta iteración y pasa al siguiente carácter.
            }
        }


        // ---------------------------------------------------------
        // CARACTERES ACENTUADOS EN OTRA CODIFICACIÓN
        // ---------------------------------------------------------

        if (uch == 0xE1 || uch == 0xC1 ||
            // á, Á

            uch == 0xE9 || uch == 0xC9 ||
            // é, É

            uch == 0xED || uch == 0xCD ||
            // í, Í

            uch == 0xF3 || uch == 0xD3 ||
            // ó, Ó

            uch == 0xFA || uch == 0xDA ||
            // ú, Ú

            uch == 0xFC || uch == 0xDC) {
            // ü, Ü

            r.vocales++;
            // Si encuentra una vocal acentuada, aumenta el contador.
        }

        else if (uch == 0xF1 || uch == 0xD1) {
            // Comprueba si el carácter es ñ o Ñ.

            r.consonantes++;
            // Aumenta el contador de consonantes.
        }

        else if (esVocal(ch)) {
            // Comprueba mediante la función esVocal() si el carácter
            // corresponde a una vocal normal como a, e, i, o o u.

            r.vocales++;
            // Si es vocal, aumenta el contador.
        }

        else if (esConsonante(ch)) {
            // Comprueba mediante esConsonante() si el carácter
            // corresponde a una consonante.

            r.consonantes++;
            // Si es consonante, aumenta el contador.
        }

        else {
            // Si no corresponde a vocal ni consonante,
            // se considera un carácter especial.

            r.especiales++;
            // Aumenta el contador de caracteres especiales.
        }
    }


    // ---------------------------------------------------------
    // CONTEO DE PALABRAS
    // ---------------------------------------------------------

    file.clear();
    // Limpia el estado del archivo después de haber llegado
    // al final durante el primer recorrido.
    file.seekg(0, ios::beg);
    // Devuelve la posición de lectura al comienzo del archivo.
    // Esto permite recorrerlo nuevamente.
    while (file >> palabra) {
        // Lee una palabra completa cada vez que encuentra texto
        // separado por espacios, saltos de línea o tabulaciones.

        r.palabras++;
        // Aumenta en uno el contador por cada palabra encontrada.
    }
    file.close();
    // Cierra el archivo después de terminar la lectura.
    return r;
    // Devuelve el objeto con todos los resultados obtenidos.
}

// Función para mostrar la interfaz de conteo en la consola.
void mostrarInterfazConteo(const string& rutaArchivo, const string& tituloMenu) {
    cout << "\n------------------------------------------------------------\n";
    cout << " " << tituloMenu << "\n";
    cout << "------------------------------------------------------------\n";
    cout << "Archivo analizado: " << (rutaArchivo.empty() ? "(Ninguno especificado)" : rutaArchivo) << "\n";

    // Comprueba si se proporcionó una ruta de archivo válida.
    if (rutaArchivo.empty()) {
        cout << "\nError: No se especifico ninguna ruta de archivo en la bandera -f.\n";
    } else {
        ResumenConteo r = contarTextoArchivo(rutaArchivo);
        // Llama a la función contarTextoArchivo() para obtener el resumen del conteo.
        if (!r.exito) {
            cout << "\nError: " << r.mensajeError << "\n";
        } else {
            cout << "\nResumen del conteo:\n";
            cout << " - Cantidad de vocales:             " << r.vocales << "\n";
            cout << " - Cantidad de consonantes:         " << r.consonantes << "\n";
            cout << " - Cantidad de caracteres especiales: " << r.especiales << "\n";
            cout << " - Cantidad de palabras:            " << r.palabras << "\n";
        }
    }

    cout << "\nPresione ENTER para volver...";
    cin.ignore();
    string enter;
    getline(cin, enter);
}
