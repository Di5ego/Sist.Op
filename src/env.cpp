#include "../include/env.h"

#if defined(_WIN32)
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
  #include <windows.h>
#endif

#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <cstdlib>

using namespace std;

// Mapa para almacenar las variables de entorno cargadas desde el archivo .env
static unordered_map<string, string> envMap; 

// Función para quitar espaciados, tabulaciones, saltos de línea y caracteres invisibles.
static string trim(const string& str) { 
    // Primero, encuentra el primer carácter que no sea espacio, tabulación o salto de línea.
    size_t first = str.find_first_not_of(" \t\r\n");
    // Si la cadena está vacía o solo contiene espacios (no encontró nada), retorna una cadena vacía.
    if (first == string::npos) return ""; 
    // Luego, encuentra el último carácter que no sea espacio, tabulación o salto de línea.
    size_t last = str.find_last_not_of(" \t\r\n"); 
    // Finalmente, recorta y retorna la subcadena que va desde ese primer carácter hasta el último carácter no vacío.
    return str.substr(first, (last - first + 1)); 
}

// Lee un archivo de configuración, ignora los comentarios (#) y las líneas vacías, 
// y guarda los pares de "Clave=Valor" en un diccionario (envMap) para usarlos en el programa.
void cargarVariablesEntorno(const string& rutaEnv) {
    // Intenta abrir el archivo de configuración (.env) usando la ruta proporcionada.
    ifstream file(rutaEnv); 
    // Verifica si el archivo no se pudo abrir (por ejemplo, si no existe).
    if (!file.is_open()) {
        // Si no se pudo abrir, termina la función silenciosamente sin hacer nada.
        return;
    }
    // Declara una variable string para almacenar cada línea leída del archivo.
    string line;
    
    // Inicia un bucle que lee el archivo línea por línea hasta llegar al final.
    while (getline(file, line)) {
        // Elimina los espacios en blanco, tabulaciones y saltos de línea al principio y al final de la línea.
        line = trim(line);
        // Si la línea quedó vacía o si empieza con '#' (es un comentario), la ignora y pasa a la siguiente iteración.
        if (line.empty() || line[0] == '#') continue;

        // Busca la posición del carácter '=' que separa la clave del valor.
        size_t posIgual = line.find('=');
        // Si encuentra el signo '=', procede a procesar la línea.
        if (posIgual != string::npos) {
            // Extrae la parte izquierda (desde el inicio hasta el '='), le quita los espacios y la guarda como 'clave'.
            string clave = trim(line.substr(0, posIgual));
            // Extrae la parte derecha (desde el carácter siguiente al '=' hasta el final), le quita los espacios y la guarda como 'valor'.
            string valor = trim(line.substr(posIgual + 1));
            // Verifica que la clave no esté vacía después de ser recortada.
            if (!clave.empty()) {
                // Si la clave es válida, almacena el par clave-valor en el mapa global 'envMap'.
                envMap[clave] = valor;
            }
        }
    }
    // Una vez terminada la lectura de todas las líneas, cierra el archivo.
    file.close();
}

// Función para obtener el valor de una variable de entorno, buscando primero 
// en el sistema operativo, luego en un archivo .env cargado, y devolviendo 
// un valor por defecto si no se encuentra.
string obtenerVariableEntorno(const string& clave, const string& valorPorDefecto) {
    // Obtiene la variable de entorno directamente del sistema operativo (usando getenv).
    const char* valSys = getenv(clave.c_str()); 
    // Verifica si el valor obtenido del sistema existe y no está vacío.
    if (valSys != nullptr && string(valSys).length() > 0) { 
        // Si existe en el sistema, lo convierte a string y lo retorna inmediatamente.
        return string(valSys); 
    }
  
    // Si no está en el sistema, busca la clave en el mapa (envMap) de variables cargadas desde el archivo .env.
    auto it = envMap.find(clave); 
    // Verifica si la clave se encontró en el mapa.
    if (it != envMap.end()) { 
        // Si la clave se encuentra en el mapa, retorna el valor asociado a dicha clave.
        return it->second; 
    }
    // Si la variable no se encontró en ninguno de los dos lugares, retorna el valor por defecto proporcionado.
    return valorPorDefecto; 
}

// Obtiene la carpeta donde esta el ejecutable en curso.
static std::filesystem::path carpetaDelEjecutable(const char* argv0) {
    namespace fs = std::filesystem;
    std::error_code ec;
#if defined(_WIN32)
    char buf[MAX_PATH] = {0};
    DWORD n = GetModuleFileNameA(nullptr, buf, MAX_PATH);
    if (n > 0 && n < MAX_PATH) return fs::path(buf).parent_path();
#else
    fs::path exe = fs::read_symlink("/proc/self/exe", ec);   // Linux / WSL
    if (!ec && !exe.empty()) return exe.parent_path();
#endif
    if (argv0 != nullptr && *argv0 != '\0') {
        fs::path p = fs::absolute(fs::path(argv0), ec);
        if (!ec) return p.parent_path();
    }
    return fs::path();
}

bool ubicarCarpetaDelPrograma(const char* argv0, string& carpetaUsada, const string& rutaEnv) {
    namespace fs = std::filesystem;
    std::error_code ec;
    if (fs::exists(rutaEnv, ec)) return false;          // se ejecuta desde la carpeta del proyecto
    fs::path carpeta = carpetaDelEjecutable(argv0);
    if (carpeta.empty() || !fs::exists(carpeta / rutaEnv, ec)) return false;
    fs::current_path(carpeta, ec);
    if (ec) return false;
    carpetaUsada = carpeta.string();
    return true;
}

string rutaAbsoluta(const string& ruta) {
    namespace fs = std::filesystem;
    if (ruta.empty()) return ruta;
    std::error_code ec;
    fs::path p(ruta);
    if (p.is_absolute()) return ruta;
    fs::path a = fs::absolute(p, ec);
    return ec ? ruta : a.lexically_normal().string();
}
