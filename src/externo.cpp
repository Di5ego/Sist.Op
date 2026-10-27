// ============================================================================
// Llamada a sistema para ejecutar los programas de cada entrega (ver externo.h)
// ============================================================================

#include "../include/externo.h"
#include "../include/env.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>

#if !defined(_WIN32)
  #include <sys/wait.h>
#endif

using namespace std;

static string recortar(const string& s) {
    size_t a = s.find_first_not_of(" \t\r\n\"'");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n\"'");
    return s.substr(a, b - a + 1);
}

int ejecutarProgramaExterno(const string& ejecutable, const vector<string>& argumentos) {
#if defined(_WIN32)
    // cmd /c: cada argumento entre comillas dobles y todo el comando encerrado
    // en un par de comillas exteriores para que cmd no rompa los argumentos.
    string comando = "\"\"" + ejecutable + "\"";
    for (const auto& a : argumentos) comando += " \"" + a + "\"";
    comando += "\"";
#else
    // sh: cada argumento entre comillas simples (escapando las comillas simples).
    auto entreComillas = [](const string& a) {
        string r = "'";
        for (char c : a) { if (c == '\'') r += "'\\''"; else r += c; }
        return r + "'";
    };
    string ruta = ejecutable.find('/') == string::npos ? "./" + ejecutable : ejecutable;
    string comando = entreComillas(ruta);
    for (const auto& a : argumentos) comando += " " + entreComillas(a);
#endif
    cout << flush;                       // lo escrito antes debe aparecer antes que la salida del programa
    int r = system(comando.c_str());
    if (r == -1) return -1;
#if defined(_WIN32)
    return r;
#else
    if (WIFEXITED(r)) return WEXITSTATUS(r);
    return -1;
#endif
}

// -------------------------------------------------------------------------------------------------------------------------------------------------

int llamarPrograma(const string& variable, const string& descripcion, const vector<string>& argumentos) {
    //1.2) LA LLAMADA A SISTEMA SE HACE DESDE EL MENU PRINCIPAL (menu.cpp) CON LA OPCION 8
    // El NOMBRE del ejecutable sale de la variable de entorno (INDEX_EXEC en .env)
    string ejecutable = recortar(obtenerVariableEntorno(variable)); //####################
    if (ejecutable.empty()) {
        cout << "\nError: la variable de entorno " << variable << " no esta definida "
             << "(debe indicar el ejecutable de " << descripcion << ").\n";
        return -1;
    }
    error_code ec;
    if (!filesystem::exists(ejecutable, ec)) {
        cout << "\nError: no se encontro el programa '" << ejecutable << "' (" << descripcion
             << "). Compile el proyecto (make) y revise " << variable << " en el .env.\n";
        return -1;
    }
    cout << "\n>> Llamada a sistema: " << ejecutable << " (" << descripcion << ")\n";
    // Antes de ejecutarlo se verifica que exista; si no, se informa sin cerrar el menú
    int codigo = ejecutarProgramaExterno(ejecutable, argumentos);   //####################
    if (codigo != 0) {
        cout << "\n<< " << ejecutable << " finalizo con codigo " << codigo << ".\n";
    } else {
        cout << "\n<< " << ejecutable << " finalizo correctamente. Volviendo al menu principal.\n";
    }
    return codigo;
}
