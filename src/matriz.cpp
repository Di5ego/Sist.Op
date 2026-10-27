#include "../include/matriz.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace std;

// Carga una matriz desde un archivo de texto, separando los valores por un caracter especificado.
ResultadoMatriz cargarMatriz(const string& rutaArchivo, char separador) {
    // Instancia un objeto 'ResultadoMatriz' llamado 'res' para almacenar el estado y los datos.
    ResultadoMatriz res;
    // Intenta abrir el archivo especificado en 'rutaArchivo' usando ifstream.
    ifstream file(rutaArchivo);
    // Verifica si el archivo falló al abrirse.
    if (!file.is_open()) {
        res.exito = false;
        res.error = "No se pudo abrir el archivo de matriz: " + rutaArchivo;
        return res;
    }

    // Declara un string 'line' para almacenar cada línea leída del archivo.
    string line;
    // Inicializa 'numColsEsperadas' en -1 para registrar la longitud de la primera fila leída.
    int numColsEsperadas = -1;
    
    // Inicia un ciclo while que lee el archivo línea por línea usando getline.
    while (getline(file, line)) {
        // Quitar caracteres de salto de linea
        if (!line.empty() && line.back() == '\r') line.pop_back();
        // Si la línea queda vacía tras la limpieza, salta a la siguiente iteracion
        if (line.empty()) continue;

        // Declara un vector de double 'fila' para almacenar los números de la línea actual.
        vector<double> fila;
        // Crea un stringstream 'ss' a partir de la línea para procesar los tokens individualmente.
        stringstream ss(line);
        // Declara un string 'valToken' que servirá si se usa un separador distinto al espacio.
        string valToken;
        
        // Evalúa si el separador indicado es un espacio en blanco o una tabulación.
        if (separador == ' ' || separador == '\t') {
            // Si es así, declara una variable 'val' de tipo double.
            double val;
            // Extrae directamente del stringstream los valores numéricos con el operador >>.
            while (ss >> val) {
                // Inserta cada valor numérico extraído al final del vector 'fila'.
                fila.push_back(val);
            }
            // Verifica si el flujo falló antes de llegar al final de la línea (indicativo de texto no numérico).
            if (ss.fail() && !ss.eof()) {
                // Si falló, marca 'exito' como false.
                res.exito = false;
                // Asigna un mensaje indicando error de formato o contenido no numérico.
                res.error = "Error de formato o contenido no numerico en la matriz (" + rutaArchivo + ")";
                // Cierra el archivo abierto.
                file.close();
                // Retorna el resultado con el error.
                return res;
            }
        } else {
            // Si el separador es otro carácter, usa getline con el separador personalizado sobre el stringstream.
            while (getline(ss, valToken, separador)) {
                // Si el token extraído está vacío, lo salta.
                if (valToken.empty()) continue;
                // Inicia un bloque try-catch para manejar excepciones de conversión a número.
                try {
                    // Declara una variable 'pos' para almacenar cuántos caracteres fueron procesados por stod.
                    size_t pos;
                    // Convierte el 'valToken' a double usando stod() y almacena el resultado en 'val'.
                    double val = stod(valToken, &pos);
                    // Compara si los caracteres procesados ('pos') no coinciden con la longitud total del token.
                    if (pos != valToken.length()) {
                        // Si hay caracteres extraños, marca 'exito' como false.
                        res.exito = false;
                        // Genera un mensaje detallando que el contenido del token es inválido.
                        res.error = "Carácter o contenido invalido '" + valToken + "' en la matriz (" + rutaArchivo + ")";
                        // Cierra el archivo.
                        file.close();
                        // Retorna el resultado fallido.
                        return res;
                    }
                    // Si la conversión fue totalmente exitosa, añade el valor 'val' al vector 'fila'.
                    fila.push_back(val);
                } catch (...) {
                    // Captura cualquier excepción lanzada por stod() (ej. cuando no hay ningún número en el token).
                    // Marca el proceso como fallido.
                    res.exito = false;
                    // Genera un mensaje de error especificando el token no numérico.
                    res.error = "Valor no numerico invalido '" + valToken + "' en la matriz (" + rutaArchivo + ")";
                    // Cierra el archivo de texto.
                    file.close();
                    // Retorna el objeto 'res' con el error.
                    return res;
                }
            }
        }

        // Terminada la lectura de la línea, si el vector 'fila' está vacío, salta a la siguiente iteración.
        if (fila.empty()) continue;

        // Si es la primera fila válida leída, guarda su tamaño como la cantidad de columnas esperadas para el resto.
        if (numColsEsperadas == -1) {
            numColsEsperadas = static_cast<int>(fila.size());
        } else if (static_cast<int>(fila.size()) != numColsEsperadas) {
            // Si no es la primera, compara el tamaño de la 'fila' actual con las columnas esperadas.
            // Si difieren, marca la operación como fallida.
            res.exito = false;
            // Asigna un mensaje de error por inconsistencia de dimensiones (matriz no rectangular).
            res.error = "Inconsistencia de dimensiones en la matriz (" + rutaArchivo + "): filas con distinto numero de columnas.";
            // Cierra el archivo.
            file.close();
            // Retorna el resultado.
            return res;
        }

        // Agrega la 'fila' validada a la matriz principal ('res.datos').
        res.datos.push_back(fila);
    }

    // Una vez leído todo el archivo, cierra el flujo de lectura.
    file.close();

    // Verifica si no se insertó ninguna fila a la matriz.
    if (res.datos.empty()) {
        // Si está vacía, marca error y asume que el archivo no tenía datos válidos.
        res.exito = false;
        // Asigna el mensaje correspondiente y retorna el error.
        res.error = "El archivo de matriz esta vacio o no contiene datos validos: " + rutaArchivo;
        return res;
    }

    // Establece la cantidad de filas de 'res' basándose en el tamaño final de 'res.datos'.
    res.filas = static_cast<int>(res.datos.size());
    // Establece la cantidad de columnas con el valor de 'numColsEsperadas'.
    res.columnas = numColsEsperadas;
    // Marca 'exito' como true indicando que la carga y validación fue completamente exitosa.
    res.exito = true;
    // Retorna el objeto 'res' con la matriz cargada.
    return res;
}

// Multiplica dos matrices (A de NxM y B de MxP), validando que las columnas de A
// coincidan con las filas de B y calculando la matriz resultante C (NxP).
ResultadoMatriz multiplicarMatrices(const Matriz& A, const Matriz& B) {
    // Instancia un objeto 'ResultadoMatriz' llamado 'res'.
    ResultadoMatriz res;
    // Obtiene y castea a entero la cantidad de filas de la matriz A.
    int filasA = static_cast<int>(A.size());
    // Obtiene y castea a entero la cantidad de columnas de A accediendo a la primera fila.
    int colsA = static_cast<int>(A[0].size());
    // Obtiene y castea a entero la cantidad de filas de la matriz B.
    int filasB = static_cast<int>(B.size());
    // Obtiene y castea a entero la cantidad de columnas de B accediendo a la primera fila.
    int colsB = static_cast<int>(B[0].size());

    // Verifica la regla matemática de multiplicación: las columnas de A deben ser iguales a las filas de B.
    if (colsA != filasB) {
        // Si no coinciden, marca el estado de la operación como fallido (false).
        res.exito = false;
        // Construye un mensaje de error descriptivo detallando las dimensiones incompatibles.
        res.error = "Incompatibilidad de dimensiones: No se pueden multiplicar las matrices (" 
                    + to_string(filasA) + "x" + to_string(colsA) + ") y (" 
                    + to_string(filasB) + "x" + to_string(colsB) 
                    + "). Las columnas de A deben ser iguales a las filas de B.";
        // Retorna el resultado con el error.
        return res;
    }

    // Si coinciden, inicializa 'res.datos' como una matriz bidimensional (vector de vectores) de tamaño filasA x colsB, llenándola de ceros.
    res.datos = Matriz(filasA, vector<double>(colsB, 0.0));
    // Inicia el bucle exterior 'i' que recorre cada fila de la matriz A.
    for (int i = 0; i < filasA; ++i) {
        // Inicia el bucle intermedio 'j' que recorre cada columna de la matriz B.
        for (int j = 0; j < colsB; ++j) {
            // Inicializa una variable local 'suma' en 0.0 para acumular el producto punto.
            double suma = 0.0;
            // Inicia el bucle interior 'k' que recorre los elementos correspondientes (columnas de A / filas de B).
            for (int k = 0; k < colsA; ++k) {
                // Multiplica el elemento A[i][k] por B[k][j] y lo suma al acumulador.
                suma += A[i][k] * B[k][j];
            }
            // Guarda el resultado del producto punto ('suma') en la celda [i][j] de la matriz resultante.
            res.datos[i][j] = suma;
        }
    }

    // Asigna a 'res.filas' el número de filas de A (N).
    res.filas = filasA;
    // Asigna a 'res.columnas' el número de columnas de B (P).
    res.columnas = colsB;
    // Marca el atributo 'exito' como verdadero.
    res.exito = true;
    // Retorna el objeto 'res' con la matriz C calculada.
    return res;
}

// Imprime en consola una matriz formateada con alineacion tabular y dos decimales.
void imprimirMatriz(const Matriz& M) {
    // Recorre cada 'fila' de la matriz proporcionada usando un bucle for-each basado en rango.
    for (const auto& fila : M) {
        // Imprime el corchete de apertura "[ " al inicio de cada fila.
        cout << "[ ";
        // Recorre cada valor numérico 'val' dentro de la 'fila' actual.
        for (double val : fila) {
            // Imprime el valor asignándole un ancho fijo de 10 espacios (setw), notación decimal fija (fixed), precisión de 2 decimales y un espacio extra al final.
            cout << setw(10) << fixed << setprecision(2) << val << " ";
        }
        // Imprime el corchete de cierre "]" seguido de un salto de línea al finalizar la fila.
        cout << "]\n";
    }
}