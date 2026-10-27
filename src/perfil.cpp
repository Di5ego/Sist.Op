#include "../include/perfil.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace std;

// Carga perfiles desde un archivo leyendo el struct completo en un solo bloque binario.
// Referencia requerida en la pauta: https://stackoverflow.com/q/74452768
bool cargarPerfilesDesdeArchivo(ListaPerfiles& lista, const string& rutaArchivo) {
    ifstream file(rutaArchivo, ios::in | ios::binary);
    if (!file.is_open()) {
        cerr << "Advertencia: No se pudo abrir el archivo de perfiles: " << rutaArchivo << endl;
        lista.cargadoEnMemoria = true;
        return false;
    }

    lista.perfiles.clear();

    // Deteccion y migracion transparente si el archivo en disco aun conserva el formato de texto anterior con ';'
    char primerByte = file.peek();
    if (primerByte != EOF && (isalpha(static_cast<unsigned char>(primerByte)) || primerByte == ';')) {
        string primeraLinea;
        streampos posInicial = file.tellg();
        if (getline(file, primeraLinea) && primeraLinea.find(';') != string::npos) {
            // Archivo de texto detectado: cargamos los perfiles existentes
            file.clear();
            file.seekg(0, ios::beg);
            string line;
            while (getline(file, line)) {
                if (line.empty()) continue;
                stringstream ss(line);
                string nombre, opcionesStr;
                if (getline(ss, nombre, ';') && getline(ss, opcionesStr)) {
                    if (!opcionesStr.empty() && opcionesStr.back() == '\r') opcionesStr.pop_back();
                    Perfil p;
                    p.setNombre(nombre);
                    stringstream ssOps(opcionesStr);
                    string opToken;
                    while (getline(ssOps, opToken, ',')) {
                        try {
                            p.agregarOpcion(stoi(opToken));
                        } catch (...) {}
                    }
                    lista.perfiles.push_back(p);
                }
            }
            file.close();

            // Guardar en el nuevo formato binario de bloque struct completo
            ofstream fileOut(rutaArchivo, ios::out | ios::trunc | ios::binary);
            if (fileOut.is_open()) {
                for (const auto& p : lista.perfiles) {
                    fileOut.write((const char*)&p, sizeof(Perfil));
                }
                fileOut.close();
            }
            lista.cargadoEnMemoria = true;
            return true;
        }
        file.clear();
        file.seekg(posInicial);
    }

    // Lectura del struct en un solo bloque mediante read((char*)&loadStruct, sizeof(loadStruct))
    // tal como se especifica en la pauta oficial y el ejemplo de referencia
    Perfil loadStruct;
    while (!file.eof()) {
        if (file.read((char*)&loadStruct, sizeof(loadStruct))) {
            lista.perfiles.push_back(loadStruct);
        }
    }

    file.close();
    lista.cargadoEnMemoria = true;
    return true;
}

// Guarda un nuevo perfil escribiendo el struct completo en un solo bloque en el archivo
bool guardarPerfil(ListaPerfiles& lista, const Perfil& nuevoPerfil, const string& rutaArchivo) {
    lista.perfiles.push_back(nuevoPerfil);

    ofstream file(rutaArchivo, ios::out | ios::app | ios::binary);
    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir el archivo para guardar el perfil." << endl;
        return false;
    }

    file.write((const char*)&nuevoPerfil, sizeof(Perfil));
    file.close();
    return true;
}

// Muestra en consola una tabla con todos los perfiles registrados y sus opciones
void listarPerfiles(ListaPerfiles& lista, const string& rutaArchivo) {
    if (!lista.cargadoEnMemoria) {
        cargarPerfilesDesdeArchivo(lista, rutaArchivo);
    }

    cout << "\n------------------------------------------------------------\n";
    cout << "                    LISTA DE PERFILES                       \n";
    cout << "------------------------------------------------------------\n";
    cout << left << setw(20) << "Nombre Perfil"
         << "Opciones Permitidas (IDs)\n";
    cout << "------------------------------------------------------------\n";
    if (lista.perfiles.empty()) {
        cout << "(No hay perfiles registrados)\n";
    } else {
        for (const auto& p : lista.perfiles) {
            cout << left << setw(20) << p.nombre;
            string opsStr = "";
            for (int i = 0; i < p.cantidadOpciones; ++i) {
                opsStr += to_string(p.opciones[i]);
                if (i + 1 < p.cantidadOpciones) opsStr += ", ";
            }
            cout << opsStr << "\n";
        }
    }
    cout << "------------------------------------------------------------\n";
}

// Busca y elimina un perfil específico por nombre, actualizando memoria y archivo con structs en bloque
bool eliminarPerfilPorNombre(ListaPerfiles& lista, const string& nombrePerfil, const string& rutaArchivo) {
    if (!lista.cargadoEnMemoria) {
        cargarPerfilesDesdeArchivo(lista, rutaArchivo);
    }

    auto it = find_if(lista.perfiles.begin(), lista.perfiles.end(), [&nombrePerfil](const Perfil& p) {
        return string(p.nombre) == nombrePerfil;
    });

    if (it == lista.perfiles.end()) {
        cout << "Error: No se encontró ningún perfil llamado '" << nombrePerfil << "'.\n";
        return false;
    }

    lista.perfiles.erase(it);

    ofstream file(rutaArchivo, ios::out | ios::trunc | ios::binary);
    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir el archivo para actualizar la lista de perfiles" << endl;
        return false;
    }

    for (const auto& p : lista.perfiles) {
        file.write((const char*)&p, sizeof(Perfil));
    }

    file.close();
    return true;
}

// Verifica si un perfil con el nombre especificado ya está registrado
bool existePerfil(const ListaPerfiles& lista, const string& nombrePerfil) {
    for (const auto& p : lista.perfiles) {
        if (string(p.nombre) == nombrePerfil) return true;
    }
    return false;
}

// Verifica si un perfil tiene permisos para ejecutar una opción del menú
// La opción 0 (Salir) siempre está permitida.
// La opción 1 (Admin) está estrictamente restringida al perfil ADMIN.
bool perfilPermiteOpcion(const Perfil& perfil, int opcion) {
    if (opcion == 0) return true;
    if (opcion == 1 && string(perfil.nombre) != "ADMIN") return false;
    for (int i = 0; i < perfil.cantidadOpciones; ++i) {
        if (perfil.opciones[i] == opcion) return true;
    }
    return false;
}