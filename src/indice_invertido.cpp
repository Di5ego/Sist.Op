// ============================================================================
// ENTREGA 3 - PROBLEMA 1: Creacion concurrente/paralela de un indice invertido
// ----------------------------------------------------------------------------
//
//   [thread principal]                       [N_THREADS threads trabajadores]
//    1. valida variables de entorno
//    2. valida carpeta y detecta libros
//    3. asigna ID unico a cada libro y
//       guarda el mapeo ID;nombre
//    4. crea los N threads  ------------->   esperan bloqueados en la cola
//    5. PRODUCTOR: encola cada libro  ---->  CONSUMIDORES: cada thread
//       (senalar semLibrosDisponibles)         a) toma 1 libro de la cola
//    6. encola N "pildoras de fin"             b) lee el archivo
//    7. join() de todos los threads            c) cuenta palabras (local, sin
//    8. verifica que no quede trabajo             sincronizacion)
//    9. escribe el indice invertido            d) integra su resultado parcial
//                                                 al indice global (seccion
//                                                 critica protegida)
//                                              e) vuelve a (a) hasta recibir
//                                                 la pildora de fin
//
// Semaforos utilizados (todos efectivos para evitar condiciones de carrera):
//   semMutexCola          (binario, valor inicial 1) exclusion mutua sobre la
//                          estructura std::queue de la cola de trabajo.
//   semLibrosDisponibles  (contador, valor inicial 0) cantidad de elementos
//                          en la cola. Un thread sin trabajo se BLOQUEA en el
//                          (no hace espera activa) hasta que haya un libro.
//   semIndice             (binario, valor inicial 1) protege el indice global
//                          y los contadores/errores compartidos.
//   semConsola            (binario, valor inicial 1) evita que los mensajes de
//                          distintos threads se mezclen en pantalla.
// ============================================================================

#include "../include/indice_invertido.h"
#include "../include/semaforo.h"
#include "../include/env.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <queue>
#include <sstream>
#include <thread>

using namespace std;
namespace fs = std::filesystem;

// Limite superior de seguridad para N_THREADS (evita valores absurdos como
// 1000000 que agotarian los recursos del sistema). No es la cantidad de
// threads: esa SIEMPRE se lee desde la variable de entorno N_THREADS.
static const int MAX_THREADS_PERMITIDOS = 1024;

// ----------------------------------------------------------------------------
// Utilidades
// ----------------------------------------------------------------------------

static string recortar(const string& s) {
    size_t a = s.find_first_not_of(" \t\r\n\"'");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n\"'");
    return s.substr(a, b - a + 1);
}

static string aMinusculasAscii(string s) {
    for (char& c : s) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    return s;
}

// ----------------------------------------------------------------------------
// 1. Variables de entorno
// ----------------------------------------------------------------------------

bool cargarConfigIndice(ConfigIndice& cfg, string& error) {
    cfg.carpetaLibros = recortar(obtenerVariableEntorno("BOOKS_DIR"));
    cfg.archivoIndice = recortar(obtenerVariableEntorno("INVERTED_INDEX_FILE"));
    cfg.archivoMapeo  = recortar(obtenerVariableEntorno("BOOK_MAP_FILE"));
    string nThreadsStr = recortar(obtenerVariableEntorno("N_THREADS"));

    if (cfg.archivoIndice.empty()) {
        error = "La variable de entorno INVERTED_INDEX_FILE no esta definida (debe indicar el archivo de salida, ej: indice_invertido.idx).";
        return false;
    }
    if (cfg.archivoMapeo.empty()) {
        error = "La variable de entorno BOOK_MAP_FILE no esta definida (debe indicar el archivo de mapeo ID;libro, ej: libros_mapeo.txt).";
        return false;
    }
    if (cfg.carpetaLibros.empty()) {
        error = "La variable de entorno BOOKS_DIR no esta definida (debe indicar la carpeta de libros, ej: LIBROS).";
        return false;
    }
    if (nThreadsStr.empty()) {
        error = "La variable de entorno N_THREADS no esta definida (debe indicar la cantidad de threads, ej: N_THREADS=4).";
        return false;
    }

    // N_THREADS debe ser un entero positivo escrito solo con digitos.
    bool soloDigitos = all_of(nThreadsStr.begin(), nThreadsStr.end(),
                              [](char c) { return isdigit(static_cast<unsigned char>(c)) != 0; });
    if (!soloDigitos || nThreadsStr.size() > 9) {
        error = "N_THREADS='" + nThreadsStr + "' no es valido. Debe ser un numero entero entre 1 y "
                + to_string(MAX_THREADS_PERMITIDOS) + ".";
        return false;
    }

// -------------------------------------------------------------------------------------------------------------------------------------------------

    int n = stoi(nThreadsStr);
    // 1.5) VALIDAR LA CONFIGURACION
    // N_THREADS se valida antes de usarlo. No hay ningún número de threads escrito en el código: el valor siempre viene del .env.
    if (n < 1 || n > MAX_THREADS_PERMITIDOS) {  //###################
        error = "N_THREADS=" + nThreadsStr + " esta fuera de rango. Debe estar entre 1 y "
                + to_string(MAX_THREADS_PERMITIDOS) + ".";
        return false;
    }
    cfg.nThreads = n;   //#########################

    // Los dos archivos de salida no pueden ser el mismo.
    error_code ec;
    if (fs::path(cfg.archivoIndice).lexically_normal() == fs::path(cfg.archivoMapeo).lexically_normal()) {
        error = "INVERTED_INDEX_FILE y BOOK_MAP_FILE apuntan al mismo archivo ('" + cfg.archivoIndice + "'). Deben ser distintos.";
        return false;
    }
    if (fs::is_directory(cfg.archivoIndice, ec)) {
        error = "INVERTED_INDEX_FILE='" + cfg.archivoIndice + "' es una carpeta, no un archivo.";
        return false;
    }
    if (fs::is_directory(cfg.archivoMapeo, ec)) {
        error = "BOOK_MAP_FILE='" + cfg.archivoMapeo + "' es una carpeta, no un archivo.";
        return false;
    }
    return true;
}

// ----------------------------------------------------------------------------
// 2. Conteo de palabras
// ----------------------------------------------------------------------------
// Una palabra es una secuencia maxima de letras. Se consideran letras:
//   - a..z / A..Z
//   - letras latinas acentuadas (a con tilde, n con virgulilla, u con dieresis,
//     etc.: rango Latin-1 U+00C0..U+00FF excepto los signos x y division).
// Todo lo demas (espacios, digitos, puntuacion, comillas tipograficas, guiones
// largos...) separa palabras. Las palabras se guardan en minusculas y en
// UTF-8, de modo que "Corazon", "CORAZON" y "corazon" son la misma palabra.
// Se aceptan archivos UTF-8 y tambien Latin-1/Windows-1252 (bytes sueltos).

static inline bool esLetraLatina(unsigned int cp) {
    return cp >= 0xC0 && cp <= 0xFF && cp != 0xD7 && cp != 0xF7;
}

static inline unsigned int minusculaLatina(unsigned int cp) {

    if (cp >= 0xC0 && cp <= 0xDE && cp != 0xD7) return cp + 0x20;
    return cp;
}

static inline void agregarUtf8(string& s, unsigned int cp) {
    s += static_cast<char>(0xC0 | (cp >> 6));
    s += static_cast<char>(0x80 | (cp & 0x3F));
}

static inline bool esContinuacion(unsigned char c) { return (c & 0xC0) == 0x80; }

// -------------------------------------------------------------------------------------------------------------------------------------------------

//1.11) EL CONTEO DE PALABRAS
// Una palabra es una secuencia de letras. Todo lo demás separa palabras. Se guarda en minúsculas: "Corazón", "CORAZÓN" y 
//"corazón" son la misma. LIMITACIÓN: solo reconoce letras del rango Latin-1 (À a ÿ).
long long contarPalabrasTexto(const string& texto, unordered_map<string, int>& conteo) {
    long long total = 0;
    string palabra;
    palabra.reserve(64);

    auto cerrarPalabra = [&]() {
        if (!palabra.empty()) {
            ++conteo[palabra];
            ++total;
            palabra.clear();
        }
    };

    const size_t n = texto.size();
    size_t i = 0;
    while (i < n) {
        unsigned char c = static_cast<unsigned char>(texto[i]);

        if (c < 0x80) {     // ASCII
            if (isalpha(c)) palabra += static_cast<char>(tolower(c));
            else cerrarPalabra();
            ++i;
        } else if (c >= 0xC2 && c <= 0xDF && i + 1 < n &&       // UTF-8 de 2 bytes
                   esContinuacion(static_cast<unsigned char>(texto[i + 1]))) {
            unsigned int cp = ((c & 0x1Fu) << 6) | (static_cast<unsigned char>(texto[i + 1]) & 0x3Fu);
            if (esLetraLatina(cp)) agregarUtf8(palabra, minusculaLatina(cp));
            else cerrarPalabra();
            i += 2;
        } else if (c >= 0xE0 && c <= 0xEF && i + 2 < n &&
                   esContinuacion(static_cast<unsigned char>(texto[i + 1])) &&
                   esContinuacion(static_cast<unsigned char>(texto[i + 2]))) {
            cerrarPalabra();
            i += 3;
        } else if (c >= 0xF0 && c <= 0xF4 && i + 3 < n &&
                   esContinuacion(static_cast<unsigned char>(texto[i + 1])) &&
                   esContinuacion(static_cast<unsigned char>(texto[i + 2])) &&
                   esContinuacion(static_cast<unsigned char>(texto[i + 3]))) {
            cerrarPalabra();
            i += 4;
        } else {
            if (esLetraLatina(c)) agregarUtf8(palabra, minusculaLatina(c));
            else cerrarPalabra();
            ++i;
        }
    }
    cerrarPalabra();
    return total;
}

// ----------------------------------------------------------------------------
// 3. Cola de trabajo compartida (productor / consumidores)
// ----------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------------------------------------------------------

class ColaTrabajo {
public:
    //1.9) LA COLA DE TRABAJO, EL CORAZON DE LA DISTRIBUCION
    ColaTrabajo() : semMutexCola(1), semLibrosDisponibles(0) {}

    
    void encolar(const LibroItem& item) {
        {
            SeccionCritica sc(semMutexCola);    // ── SECCIÓN CRÍTICA 1
            elementos.push(item);
        }                                      
        semLibrosDisponibles.senalar();        // V: hay un libro más
    }
    // CONSUMIDOR: el thread se bloquea AQUÍ si la cola está vacía (sin gastar CPU).
    // ORDEN IMPORTANTE: primero esperamos el libro (semLibrosDisponibles) y solo después tomamos el candado (semMutexCola). 
    // Así nadie se queda con el candado tomado mientras espera.
    LibroItem desencolar() {
        semLibrosDisponibles.esperar();        // P: espera a que haya un libro
        SeccionCritica sc(semMutexCola);       // ── SECCIÓN CRÍTICA 1
        LibroItem item = elementos.front();

        elementos.pop(); // Sección crítica 1: sacar el libro. Como ocurre con el candado tomado, DOS threads NUNCA pueden sacar el mismo libro
        
        return item;                           
    }

    size_t pendientes() {
        SeccionCritica sc(semMutexCola);
        return elementos.size();
    }

private:
    queue<LibroItem> elementos;   // recurso compartido
    Semaforo semMutexCola;        // binario: exclusion mutua sobre 'elementos'
    Semaforo semLibrosDisponibles;// contador: elementos presentes en la cola
};

// Una entrada del indice: (id del libro, cantidad de apariciones)
typedef vector<pair<int, int>> ListaApariciones;

// Estado compartido por todos los threads trabajadores.
struct ContextoProceso {
    ColaTrabajo cola;

    Semaforo semIndice{1};                             // protege todo lo de abajo
    unordered_map<string, ListaApariciones> indice;    // indice invertido global
    int librosProcesados = 0;
    int librosConError = 0;
    long long palabrasTotales = 0;
    vector<string> errores;

    Semaforo semConsola{1};                            // protege la salida por pantalla
    bool verbose = true;
    int totalLibros = 0;

    // Cada thread escribe SOLO en su propia posicion -> no requiere semaforo.
    vector<int> librosPorThread;
};

// ----------------------------------------------------------------------------
// 4. Thread trabajador
// ----------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------------------------------------------------------

static void threadTrabajador(int idThread, ContextoProceso* ctx) {
    //1.10) EL THREAD TRABAJADOR
    // CADA THREAD repite este ciclo hasta recibir la píldora de fin. Es el mismo código en los N threads: por eso el trabajo se reparte solo.
    while (true) {
        LibroItem libro = ctx->cola.desencolar();   // (a) obtener un libro desde la cola (bloqueante)
        if (libro.id == 0) break;   // pildora de fin: no quedan libros

        // (b) procesar el contenido del libro (leer el archivo completo)
        string contenido;
        bool lecturaOk = false;
        {
            ifstream archivo(libro.rutaCompleta, ios::in | ios::binary);
            if (archivo.is_open()) {
                ostringstream ss;
                ss << archivo.rdbuf();
                contenido = ss.str();
                lecturaOk = !archivo.bad();
            }
        }

        if (!lecturaOk) {
            SeccionCritica sc(ctx->semIndice);
            ctx->librosConError++;
            ctx->errores.push_back("Libro " + to_string(libro.id) + " (" + libro.nombreArchivo + "): no se pudo leer.");
            continue;
        }

        // (c) PARALELO SIN CANDADO: el conteo usa un mapa PROPIO de este thread. Ningún otro thread lo ve, así que aquí no 
        // hace falta semáforo. Es lo más costoso y es donde realmente se gana tiempo.
        unordered_map<string, int> conteoLocal;
        conteoLocal.reserve(32768);
        long long palabrasLibro = contarPalabrasTexto(contenido, conteoLocal);
        contenido.clear();
        contenido.shrink_to_fit();

        // (d) incorporar el resultado parcial al indice invertido global.
        //     SECCION CRITICA: varios threads modificarian el mismo unordered_map a la vez (rehash, insercion de la misma palabra),
        //     lo que corromperia la estructura. Se protege con semIndice.
        int procesadosHastaAhora;
        {
            // SECCIÓN CRÍTICA 2: el índice global lo comparten todos los threads. Dos inserciones simultáneas podrían corromper el 
            //unordered_map (un rehash a mitad de otra inserción). El semáforo evita eso.
            SeccionCritica sc(ctx->semIndice);  
            for (const auto& par : conteoLocal) {
                ctx->indice[par.first].emplace_back(libro.id, par.second);
            }
            ctx->librosProcesados++;
            ctx->palabrasTotales += palabrasLibro;
            procesadosHastaAhora = ctx->librosProcesados + ctx->librosConError;
        }                                        

        ctx->librosPorThread[idThread - 1]++;

        if (ctx->verbose) {
            SeccionCritica sc(ctx->semConsola);  
            cout << "  [Thread " << setw(2) << idThread << "] libro " << setw(3) << libro.id
                 << " (" << libro.nombreArchivo << "): " << palabrasLibro << " palabras, "
                 << conteoLocal.size() << " distintas  [" << procesadosHastaAhora << "/"
                 << ctx->totalLibros << "]\n";
        }

    }

    if (ctx->verbose) {
        SeccionCritica sc(ctx->semConsola);
        cout << "  [Thread " << setw(2) << idThread << "] sin libros pendientes: finaliza ("
             << ctx->librosPorThread[idThread - 1] << " libros procesados).\n";
    }   // (e) vuelve al inicio: pide otro libro
}

// ----------------------------------------------------------------------------
// 5. Proceso completo
// ----------------------------------------------------------------------------

IndiceInvertidoEstadisticas ejecutarCreacionIndiceInvertido(bool verbose) {
    IndiceInvertidoEstadisticas est;
    ConfigIndice cfg;
    string error;

    // ---- 5.1 Validar variables de entorno ----
    if (!cargarConfigIndice(cfg, error)) {
        est.mensajeError = error;
        return est;
    }
    est.rutaIndice = cfg.archivoIndice;
    est.rutaMapeo = cfg.archivoMapeo;

    if (verbose) {
        cout << "\nConfiguracion (variables de entorno):\n";
        cout << "  BOOKS_DIR           = " << cfg.carpetaLibros << "\n";
        cout << "  INVERTED_INDEX_FILE = " << cfg.archivoIndice << "\n";
        cout << "  BOOK_MAP_FILE       = " << cfg.archivoMapeo << "\n";
        cout << "  N_THREADS           = " << cfg.nThreads << "\n";
    }

    // ---- 5.2 Verificar la carpeta de libros ----
    error_code ec;
    fs::path carpeta(cfg.carpetaLibros);
    if (!fs::exists(carpeta, ec)) {
        est.mensajeError = "La carpeta de libros '" + cfg.carpetaLibros + "' no existe (revise BOOKS_DIR).";
        return est;
    }
    if (!fs::is_directory(carpeta, ec)) {
        est.mensajeError = "'" + cfg.carpetaLibros + "' existe pero no es una carpeta (revise BOOKS_DIR).";
        return est;
    }

    vector<pair<string, string>> archivos;  
    int ignorados = 0;
    fs::directory_iterator it(carpeta, ec);
    if (ec) {
        est.mensajeError = "No se puede leer la carpeta '" + cfg.carpetaLibros + "': " + ec.message() + ".";
        return est;
    }
    for (; it != fs::directory_iterator(); it.increment(ec)) {
        if (ec) {
            est.mensajeError = "Error leyendo la carpeta '" + cfg.carpetaLibros + "': " + ec.message() + ".";
            return est;
        }
        error_code ec2;
        if (!it->is_regular_file(ec2)) continue;
        string nombre = it->path().filename().string();
        if (aMinusculasAscii(it->path().extension().string()) != ".txt") { ignorados++; continue; }
        archivos.emplace_back(nombre, it->path().string());
    }

    vector<pair<string, string>> legibles;
    vector<string> noLegibles;
    for (const auto& a : archivos) {
        ifstream prueba(a.second, ios::in | ios::binary);
        if (prueba.is_open()) legibles.push_back(a);
        else noLegibles.push_back(a.first);
    }

    if (archivos.empty()) {
        est.mensajeError = "La carpeta '" + cfg.carpetaLibros + "' no contiene archivos .txt para procesar.";
        return est;
    }
    if (legibles.empty()) {
        est.mensajeError = "Ninguno de los " + to_string(archivos.size()) + " archivos .txt de '" + cfg.carpetaLibros
                           + "' puede leerse (revise los permisos).";
        return est;
    }

// -------------------------------------------------------------------------------------------------------------------------------------------------

    // 1.6)CARPETA DE LIBROS Y ASIGNACION DE IDs
    // PASO 1: identificar los libros. Se ordenan alfabéticamente para que los IDs salgan siempre iguales entre ejecuciones
    sort(legibles.begin(), legibles.end()); // orden alfabético

    if (verbose) {
        cout << "\nCarpeta '" << cfg.carpetaLibros << "' verificada: existe y se puede leer.\n";
        cout << "  Libros (.txt) identificados para procesar: " << legibles.size() << "\n";
        if (ignorados > 0) cout << "  Archivos ignorados (no son .txt): " << ignorados << "\n";
        for (const auto& nl : noLegibles) cout << "  ADVERTENCIA: no se puede leer '" << nl << "', se omitira.\n";
    }

    // ---- 5.3 Asignar ID unico a cada libro y guardar el mapeo ID;nombre ----
    vector<LibroItem> libros;
    for (size_t i = 0; i < legibles.size(); ++i) {
        LibroItem li;
        // PASO 2: cada libro recibe un ID numérico único (1..N).
        li.id = static_cast<int>(i) + 1;    // ID único por libro
        li.nombreArchivo = legibles[i].first;
        li.rutaCompleta = legibles[i].second;
        libros.push_back(li);
    }

    {
        ofstream prueba(cfg.archivoIndice, ios::out | ios::app);
        if (!prueba.is_open()) {
            est.mensajeError = "No se puede escribir el archivo de indice '" + cfg.archivoIndice
                               + "' (revise INVERTED_INDEX_FILE y los permisos).";
            return est;
        }
    }
    {
        ofstream mapeo(cfg.archivoMapeo, ios::out | ios::trunc | ios::binary);
        if (!mapeo.is_open()) {
            est.mensajeError = "No se pudo crear el archivo de mapeo '" + cfg.archivoMapeo + "' (revise BOOK_MAP_FILE y los permisos).";
            return est;
        }
        // PASO 3: se guarda la asociación ID;nombre en BOOK_MAP_FILE, para poder traducir después los IDs del índice a nombres de libros
        for (const auto& li : libros) mapeo << li.id << ';' << li.nombreArchivo << '\n';
        if (!mapeo.good()) {
            est.mensajeError = "Error escribiendo el archivo de mapeo '" + cfg.archivoMapeo + "'.";
            return est;
        }
    }
    if (verbose) cout << "  Mapeo ID <-> libro guardado en '" << cfg.archivoMapeo << "'.\n";

    est.librosEncontrados = static_cast<int>(libros.size());

    // ---- 5.4 Crear threads, llenar la cola y esperar ----
    ContextoProceso ctx;
    ctx.verbose = verbose;
    ctx.totalLibros = est.librosEncontrados;
    ctx.librosPorThread.assign(cfg.nThreads, 0);
    ctx.indice.reserve(1 << 18);

    if (verbose) {
        cout << "\nCreando " << cfg.nThreads << " threads trabajadores y encolando "
             << libros.size() << " libros...\n\n";
    }

    auto inicio = chrono::steady_clock::now();

    vector<thread> threads;
    threads.reserve(cfg.nThreads);
    try {

// -------------------------------------------------------------------------------------------------------------------------------------------------

        // 1.7) CREACION DE THREADS TRABAJADORES
        // PASO 4: se crean N_THREADS threads ANTES de llenar la cola. Al arrancar no hay libros, así que quedan 
        //bloqueados esperando trabajo.
        for (int t = 1; t <= cfg.nThreads; ++t) {
            threads.emplace_back(threadTrabajador, t, &ctx);
        }
    } catch (const exception& e) {
        for (size_t k = 0; k < threads.size(); ++k) ctx.cola.encolar(LibroItem());
        for (auto& th : threads) th.join();     // espera a TODOS los threads
        est.mensajeError = string("No se pudieron crear los ") + to_string(cfg.nThreads)
                           + " threads solicitados (" + e.what() + "). Pruebe con un N_THREADS menor.";
        return est;     // NO escribe un índice incompleto
    }

// -------------------------------------------------------------------------------------------------------------------------------------------------

    // 1.8)EL PRODUCTOR LLENA LA COLA DE TRABAJO
    //// PASO 5 (PRODUCTOR): el hilo principal encola cada libro. Cada encolado despierta a un thread que estaba esperando.
    for (const auto& li : libros) ctx.cola.encolar(li);
    // PASO 6: una "píldora de fin" (ID 0) por thread. Como la cola es FIFO, las píldoras salen DESPUÉS de todos los 
    // libros: así cada thread sabe cuándo parar.
    for (int t = 0; t < cfg.nThreads; ++t) ctx.cola.encolar(LibroItem());
    
// -------------------------------------------------------------------------------------------------------------------------------------------------

    //1.12) VERIFICAR Y ESCRIBIR EL INDICE
    // PASO 7: el hilo principal ESPERA a que todos los threads terminen.
    // Ningún thread queda ejecutándose al terminar el programa.
    for (auto& th : threads) th.join();

    auto finProceso = chrono::steady_clock::now();

    // ---- 5.5 Verificacion de finalizacion ----
    size_t pendientes = ctx.cola.pendientes();
    int completados = ctx.librosProcesados + ctx.librosConError;
    if (verbose) {
        cout << "\nVerificacion de finalizacion:\n";
        cout << "  - Libros pendientes en la cola: " << pendientes << "\n";
        cout << "  - Libros procesados: " << ctx.librosProcesados << " de " << est.librosEncontrados
             << (ctx.librosConError ? " (" + to_string(ctx.librosConError) + " con error)" : string("")) << "\n";
        cout << "  - Threads finalizados: " << threads.size() << " de " << cfg.nThreads << "\n";
    }
    // PASO 8: verificación de finalización. Cola vacía + todos los libros procesados + threads terminados. Solo entonces se construye el índice.
    if (pendientes != 0 || completados != est.librosEncontrados) {
        est.mensajeError = "El procesamiento no se completo correctamente (quedaron libros sin procesar).";
        return est;
    }
    for (const auto& e : ctx.errores) cerr << "  ADVERTENCIA: " << e << "\n";

    // ---- 5.6 Construir y escribir el indice invertido unico ----
    // Cada palabra aparece una sola vez; sus apariciones se ordenan por ID de libro.
    vector<pair<const string, ListaApariciones>*> entradas;
    entradas.reserve(ctx.indice.size());
    for (auto& par : ctx.indice) {
        sort(par.second.begin(), par.second.end());
        entradas.push_back(&par);
    }
    sort(entradas.begin(), entradas.end(),
         [](const pair<const string, ListaApariciones>* a, const pair<const string, ListaApariciones>* b) {
             return a->first < b->first;
         });

    {
        ofstream salida(cfg.archivoIndice, ios::out | ios::trunc | ios::binary);
        if (!salida.is_open()) {
            est.mensajeError = "No se pudo crear el archivo de indice '" + cfg.archivoIndice + "'.";
            return est;
        }
        string linea;
        for (const auto* e : entradas) {
            linea.clear();
            // PASO 9: se escribe el índice invertido, una línea por palabra: palabra;(id_libro,cantidad);(id_libro,cantidad)...
            // Cada palabra aparece UNA sola vez, con todos los libros donde se encontró.
            linea += e->first;      // palabra
            for (const auto& ap : e->second) {
                linea += ";(";
                linea += to_string(ap.first);
                linea += ',';
                linea += to_string(ap.second);
                linea += ')';   // (id,cantidad)
            }
            linea += '\n';
            salida.write(linea.data(), static_cast<streamsize>(linea.size()));
        }
        salida.flush();
        if (!salida.good()) {
            est.mensajeError = "Error escribiendo el archivo de indice '" + cfg.archivoIndice + "'.";
            return est;
        }
    }

    auto fin = chrono::steady_clock::now();

    est.exito = true;
    est.librosProcesados = ctx.librosProcesados;
    est.librosConError = ctx.librosConError;
    est.palabrasTotales = ctx.palabrasTotales;
    est.palabrasUnicas = static_cast<long long>(entradas.size());
    est.threadsUsados = cfg.nThreads;
    est.librosPorThread = ctx.librosPorThread;
    est.tiempoSegundos = chrono::duration<double>(fin - inicio).count();
    (void)finProceso;
    return est;
}

// ----------------------------------------------------------------------------
// 6. Busqueda de una palabra en el indice ya generado (apoyo para verificar)
// ----------------------------------------------------------------------------

static void buscarPalabraEnIndice() {
    ConfigIndice cfg;
    string error;
    if (!cargarConfigIndice(cfg, error)) {
        cout << "\nError: " << error << "\n";
        return;
    }
    ifstream idx(cfg.archivoIndice, ios::in | ios::binary);
    if (!idx.is_open()) {
        cout << "\nError: no existe el archivo de indice '" << cfg.archivoIndice
             << "'. Primero cree el indice invertido.\n";
        return;
    }

    cout << "Ingrese la palabra a buscar: ";
    string palabra;
    if (!getline(cin, palabra)) return;
    palabra = recortar(palabra);
    unordered_map<string, int> norm;
    contarPalabrasTexto(palabra, norm);       
    if (norm.size() != 1) {
        cout << "Error: ingrese exactamente una palabra (solo letras).\n";
        return;
    }
    string clave = norm.begin()->first;

    unordered_map<int, string> nombres;
    ifstream mapeo(cfg.archivoMapeo, ios::in | ios::binary);
    string linea;
    while (mapeo.is_open() && getline(mapeo, linea)) {
        if (!linea.empty() && linea.back() == '\r') linea.pop_back();
        size_t p = linea.find(';');
        if (p == string::npos) continue;
        try { nombres[stoi(linea.substr(0, p))] = linea.substr(p + 1); } catch (...) {}
    }

    string prefijo = clave + ";";
    while (getline(idx, linea)) {
        if (linea.compare(0, prefijo.size(), prefijo) != 0) continue;
        cout << "\nEntrada en el indice:\n  " << linea << "\n\nDetalle:\n";
        long long total = 0;
        stringstream ss(linea.substr(prefijo.size()));
        string tok;
        while (getline(ss, tok, ';')) {
            int id = 0, cant = 0;
            if (sscanf(tok.c_str(), "(%d,%d)", &id, &cant) == 2) {
                total += cant;
                cout << "  - " << setw(6) << cant << " vez/veces en el libro " << id << " ("
                     << (nombres.count(id) ? nombres[id] : string("?")) << ")\n";
            }
        }
        cout << "  Total: " << total << " apariciones.\n";
        return;
    }
    cout << "\nLa palabra '" << clave << "' no aparece en el indice.\n";
}

// ----------------------------------------------------------------------------
// 7. Pantalla del menu: "Crear indice invertido"
// ----------------------------------------------------------------------------

static int leerOpcionIndice() {
    string linea;
    while (true) {
        cout << "\nOpcion : ";
        if (!getline(cin, linea)) return 0;
        linea = recortar(linea);
        if (!linea.empty() && all_of(linea.begin(), linea.end(),
                                     [](char c) { return isdigit(static_cast<unsigned char>(c)) != 0; })
            && linea.size() < 6) {
            return stoi(linea);
        }
        cout << "Entrada invalida. Ingrese un numero.";
    }
}

void pantallaCrearIndiceInvertido() {
    int op = -1;
    while (op != 0) {
        cout << "\n============================================================\n";
        cout << "       CREAR INDICE INVERTIDO (procesamiento paralelo)      \n";
        cout << "============================================================\n";
        cout << "1) Crear indice invertido\n";
        cout << "2) Buscar una palabra en el indice generado\n";
        cout << "0) Volver al menu principal\n";
        op = leerOpcionIndice();

        if (op == 1) {
            IndiceInvertidoEstadisticas est = ejecutarCreacionIndiceInvertido(true);
            if (!est.exito) {
                cout << "\n------------------------------------------------------------\n";
                cout << "No fue posible crear el indice invertido.\n";
                cout << "Motivo: " << est.mensajeError << "\n";
                cout << "------------------------------------------------------------\n";
                continue;
            }
            cout << "\n============================================================\n";
            cout << " INDICE INVERTIDO CREADO EXITOSAMENTE\n";
            cout << "============================================================\n";
            cout << " Libros procesados      : " << est.librosProcesados << " de " << est.librosEncontrados << "\n";
            cout << " Threads trabajadores   : " << est.threadsUsados << "\n";
            for (size_t t = 0; t < est.librosPorThread.size(); ++t) {
                cout << "    Thread " << setw(2) << (t + 1) << " -> " << est.librosPorThread[t] << " libro(s)\n";
            }
            cout << " Palabras contadas      : " << est.palabrasTotales << "\n";
            cout << " Palabras distintas     : " << est.palabrasUnicas << "\n";
            cout << " Tiempo total           : " << fixed << setprecision(3) << est.tiempoSegundos << " s\n";
            cout.unsetf(ios::fixed);
            cout << " Indice invertido       : " << est.rutaIndice << "\n";
            cout << " Mapeo ID <-> libro     : " << est.rutaMapeo << "\n";
            cout << "============================================================\n";
        } else if (op == 2) {
            buscarPalabraEnIndice();
        } else if (op != 0) {
            cout << "Opcion invalida.\n";
        }
    }
}