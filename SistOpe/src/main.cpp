#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <cstdlib>

#include "../include/usuario.h"
#include "../include/perfil.h"
#include "../include/menu.h"

using namespace std;

static unordered_map<string, string> envMap;

//Funcion para quitar espaciados, tabulaciones, saltos de linea y caracteres invisibles que puede llegar a ingresar un usuario
//Ej: "   Cristobal Mansilla   \n" -> "Cristobal Mansilla" 
static string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

//Funcion para cargar las variables del archivo .env
void cargarVariablesEntorno(const string& rutaEnv = ".env") {
    ifstream file(rutaEnv);
    if (!file.is_open()) {
        return;
    }

    string line;
    while (getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        size_t posIgual = line.find('=');
        if (posIgual != string::npos) {
            string clave = trim(line.substr(0, posIgual));
            string valor = trim(line.substr(posIgual + 1));
            if (!clave.empty()) {
                envMap[clave] = valor;
            }
        }
    }
    file.close();
}

//Funcion para obtener una variable de entorno, primero revisa en el sistema operativo nativo, luego en el archivo .env cargado, 
//y si no existe devuelve un valor por defecto
string obtenerVariableEntorno(const string& clave, const string& valorPorDefecto = "") {
    // Revisar si existe en nuestro sistema operativo
    const char* valSys = getenv(clave.c_str());
    if (valSys != nullptr && string(valSys).length() > 0) {
        return string(valSys);
    }
    // Revisar si existe en el archivo .env cargado
    auto it = envMap.find(clave);
    if (it != envMap.end()) {
        return it->second;
    }
    return valorPorDefecto;
}

int main() {

    //Cargar variables de entorno desde el archivo .env
    cargarVariablesEntorno(".env");

    //Obtener rutas de archivos segun variables de entorno USER_FILE y PERFIL_FILE
    string userFile = obtenerVariableEntorno("USER_FILE", "USUARIOS.TXT");
    string perfilFile = obtenerVariableEntorno("PERFIL_FILE", "PERFILES.TXT");

    cout << "Inicializando sistema...\n";
    cout << "USER_FILE: " << userFile << "\n";
    cout << "PERFIL_FILE: " << perfilFile << "\n";

    // Inicia los construcctores de las listas de usuarios y perfiles
    ListaUsuarios listaUsuarios;
    ListaPerfiles listaPerfiles;

    //Iniciar el menu
    mostrarMenuPrincipal(listaUsuarios, listaPerfiles, userFile, perfilFile);

    return 0;
}
