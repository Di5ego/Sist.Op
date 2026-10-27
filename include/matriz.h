#ifndef MATRIZ_H
#define MATRIZ_H

#include <string>
#include <vector>

typedef std::vector<std::vector<double>> Matriz;

// Estructura para almacenar el resultado de las operaciones con matrices
struct ResultadoMatriz {
    Matriz datos;
    int filas = 0;
    int columnas = 0;
    bool exito = false;
    std::string error = "";
};

// Carga una matriz desde un archivo indicando el separador
ResultadoMatriz cargarMatriz(const std::string& rutaArchivo, char separador);

// Multiplica la matriz A (NxM) y B (MxP) -> C (NxP)
ResultadoMatriz multiplicarMatrices(const Matriz& A, const Matriz& B);

// Imprime una matriz en consola
void imprimirMatriz(const Matriz& M);

#endif // MATRIZ_H
