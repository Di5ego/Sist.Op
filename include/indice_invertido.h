#ifndef INDICE_INVERTIDO_H
#define INDICE_INVERTIDO_H

// ============================================================================
// ENTREGA 3 - PROBLEMA 1: Creacion concurrente/paralela de un indice invertido
// ----------------------------------------------------------------------------
// Flujo:  cola de trabajo -> threads trabajadores -> semaforos -> resultados
//         parciales -> indice invertido unico (archivo INVERTED_INDEX_FILE)
//
// Variables de entorno utilizadas (ver README):
//   BOOKS_DIR            carpeta con los libros (.txt)
//   INVERTED_INDEX_FILE  nombre/path del archivo de salida (indice_invertido.idx)
//   BOOK_MAP_FILE        nombre/path del archivo de mapeo  ID;nombre_libro
//   N_THREADS            cantidad de threads trabajadores
// ============================================================================

#include <string>
#include <vector>
#include <unordered_map>

// Elemento de la cola de trabajo: representa UN libro pendiente de procesar.
// Un elemento con id == 0 es la "pildora de fin" que indica a un thread que
// ya no quedan libros y que debe terminar.
struct LibroItem {
    int id = 0;                    // identificador numerico unico del libro (1..N)
    std::string nombreArchivo;     // nombre del archivo (ej: don_quijote.txt)
    std::string rutaCompleta;      // ruta para abrir el archivo
};

// Configuracion obtenida y validada desde las variables de entorno.
struct ConfigIndice {
    std::string carpetaLibros;
    std::string archivoIndice;
    std::string archivoMapeo;
    int nThreads = 0;
};

// Estadisticas resultantes de la ejecucion concurrente.
struct IndiceInvertidoEstadisticas {
    bool exito = false;
    std::string mensajeError;
    int librosEncontrados = 0;         // libros identificados en la carpeta
    int librosProcesados = 0;          // libros procesados correctamente
    int librosConError = 0;            // libros que no pudieron leerse durante el proceso
    long long palabrasTotales = 0;     // total de ocurrencias de palabras contadas
    long long palabrasUnicas = 0;      // entradas (palabras distintas) del indice
    int threadsUsados = 0;
    std::vector<int> librosPorThread;  // cuantos libros proceso cada thread
    double tiempoSegundos = 0.0;
    std::string rutaIndice;
    std::string rutaMapeo;
};

// Lee y valida las variables de entorno necesarias. Devuelve false y llena
// 'error' con un mensaje comprensible si falta alguna o su valor es invalido.
bool cargarConfigIndice(ConfigIndice& cfg, std::string& error);

// Separa un texto en palabras normalizadas (minusculas, UTF-8) y acumula en
// 'conteo' cuantas veces aparece cada una. Devuelve el total de palabras.
// Se expone para poder probarla de forma aislada.
long long contarPalabrasTexto(const std::string& texto,
                              std::unordered_map<std::string, int>& conteo);

// Funcion principal: crea el indice invertido en paralelo usando una cola de
// trabajo compartida, N_THREADS threads trabajadores y semaforos.
// Si 'verbose' es true informa el avance por consola.
IndiceInvertidoEstadisticas ejecutarCreacionIndiceInvertido(bool verbose = true);

// Pantalla del menu principal: opcion "Crear indice invertido".
void pantallaCrearIndiceInvertido();

#endif // INDICE_INVERTIDO_H
