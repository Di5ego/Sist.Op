#include "../include/usuario.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip> 
#include <algorithm> 

using namespace std;

// Carga los usuarios desde el archivo leyendo el struct completo en un solo bloque binario.
// Referencia requerida en la pauta: https://stackoverflow.com/q/74452768
bool cargarUsuariosDesdeArchivo(ListaUsuarios& lista, const string& rutaArchivo) {
    ifstream file(rutaArchivo, ios::in | ios::binary);
    if (!file.is_open()) {
        cerr << "Advertencia: No se pudo abrir el archivo de usuarios: " << rutaArchivo << endl;
        lista.cargadoEnMemoria = true;
        return false;
    }

    lista.usuarios.clear();

    // Deteccion y migracion transparente si el archivo en disco aun conserva el formato de texto anterior con ';'
    char primerByte = file.peek();
    if (primerByte != EOF && (isdigit(static_cast<unsigned char>(primerByte)) || primerByte == ';')) {
        string primeraLinea;
        streampos posInicial = file.tellg();
        if (getline(file, primeraLinea) && primeraLinea.find(';') != string::npos) {
            // Archivo de texto detectado: cargamos los usuarios existentes
            file.clear();
            file.seekg(0, ios::beg);
            string line;
            while (getline(file, line)) {
                if (line.empty()) continue;
                stringstream ss(line);
                string idStr, nombre, username, password, perfil;
                if (getline(ss, idStr, ';') &&
                    getline(ss, nombre, ';') &&
                    getline(ss, username, ';') &&
                    getline(ss, password, ';') &&
                    getline(ss, perfil)) {
                    if (!perfil.empty() && perfil.back() == '\r') perfil.pop_back();
                    try {
                        Usuario u(stoi(idStr), nombre, username, password, perfil);
                        lista.usuarios.push_back(u);
                    } catch (...) { continue; }
                }
            }
            file.close();

            // Escribir los usuarios en el nuevo formato binario de bloque struct completo
            ofstream fileOut(rutaArchivo, ios::out | ios::trunc | ios::binary);
            if (fileOut.is_open()) {
                for (const auto& u : lista.usuarios) {
                    fileOut.write((const char*)&u, sizeof(Usuario));
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
    Usuario loadStruct;
    while (!file.eof()) {
        if (file.read((char*)&loadStruct, sizeof(loadStruct))) {
            lista.usuarios.push_back(loadStruct);
        }
    }

    file.close();
    lista.cargadoEnMemoria = true;
    return true;
}

// Guarda un nuevo usuario escribiendo el struct completo en un solo bloque en el archivo
bool guardarUsuario(ListaUsuarios& lista, const Usuario& nuevoUsuario, const string& rutaArchivo) {
    lista.usuarios.push_back(nuevoUsuario);

    // Escribe el struct completo directamente al final del archivo en modo binario
    ofstream file(rutaArchivo, ios::out | ios::app | ios::binary);
    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir el archivo para guardar el usuario." << endl;
        return false;
    }

    file.write((const char*)&nuevoUsuario, sizeof(Usuario));
    file.close();
    return true;
}

// Muestra en consola la lista formateada de usuarios registrados
void listarUsuarios(ListaUsuarios& lista, const string& rutaArchivo) {
    if (!lista.cargadoEnMemoria) {
        cargarUsuariosDesdeArchivo(lista, rutaArchivo);
    }

    cout << "\n------------------------------------------------------------\n";
    cout << "                   LISTA DE USUARIOS                        \n";
    cout << "------------------------------------------------------------\n";
    cout << left << setw(6) << "Id"
         << setw(20) << "Nombre"
         << setw(15) << "Username"
         << setw(15) << "Perfil" << "\n";
    cout << "------------------------------------------------------------\n";
    if (lista.usuarios.empty()) {
        cout << "(No hay usuarios registrados)\n";
    } else {
        for (const auto& u : lista.usuarios) {
            cout << left << setw(6) << u.id
                 << setw(20) << u.nombre
                 << setw(15) << u.username
                 << setw(15) << u.perfil << "\n";
        }
    }
    cout << "------------------------------------------------------------\n";
}

// Elimina un usuario por su ID y actualiza el archivo escribiendo los structs restantes en bloque
bool eliminarUsuarioPorId(ListaUsuarios& lista, int idEliminar, const string& rutaArchivo, bool confirmarAlertaAdmin) {
    (void)confirmarAlertaAdmin;
    if (!lista.cargadoEnMemoria) {
        cargarUsuariosDesdeArchivo(lista, rutaArchivo);
    }

    auto it = find_if(lista.usuarios.begin(), lista.usuarios.end(), [idEliminar](const Usuario& u) {
        return u.id == idEliminar;
    });

    if (it == lista.usuarios.end()) {
        cout << "Error: No se encontró ningún usuario con el ID " << idEliminar << ".\n";
        return false;
    }

    lista.usuarios.erase(it);

    // Sobrescribe el archivo en modo binario guardando cada struct completo
    ofstream file(rutaArchivo, ios::out | ios::trunc | ios::binary);
    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir el archivo para actualizar la lista de usuarios." << endl;
        return false;
    }

    for (const auto& u : lista.usuarios) {
        file.write((const char*)&u, sizeof(Usuario));
    }

    file.close();
    return true;
}

// Elimina todos los usuarios asociados a un perfil (en cascada) y actualiza el archivo en bloque
int eliminarUsuariosPorPerfil(ListaUsuarios& lista, const string& nombrePerfil, const string& rutaArchivo) {
    if (!lista.cargadoEnMemoria) {
        cargarUsuariosDesdeArchivo(lista, rutaArchivo);
    }

    size_t tamInicial = lista.usuarios.size();
    lista.usuarios.erase(
        remove_if(lista.usuarios.begin(), lista.usuarios.end(), [&nombrePerfil](const Usuario& u) {
            return string(u.perfil) == nombrePerfil;
        }),
        lista.usuarios.end()
    );

    int eliminados = static_cast<int>(tamInicial - lista.usuarios.size());
    if (eliminados > 0) {
        ofstream file(rutaArchivo, ios::out | ios::trunc | ios::binary);
        if (file.is_open()) {
            for (const auto& u : lista.usuarios) {
                file.write((const char*)&u, sizeof(Usuario));
            }
            file.close();
        }
    }
    return eliminados;
}

// Verifica si un ID de usuario ya existe en la lista de usuarios
bool existeUsuarioId(const ListaUsuarios& lista, int id) {
    for (const auto& u : lista.usuarios) {
        if (u.id == id) return true;
    }
    return false;
}

// Verifica si un username ya existe en la lista de usuarios
bool existeUsername(const ListaUsuarios& lista, const string& username) {
    for (const auto& u : lista.usuarios) {
        if (string(u.username) == username) return true;
    }
    return false;
}
