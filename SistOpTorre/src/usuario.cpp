#include "../include/usuario.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip> 
#include <algorithm> 

using namespace std;

bool cargarUsuariosDesdeArchivo(ListaUsuarios& lista, const string& rutaArchivo) {
    ifstream file(rutaArchivo);
    if (!file.is_open()) {
        cerr << "Advertencia: No se pudo abrir el archivo de usuarios: " << rutaArchivo << endl;
        lista.cargadoEnMemoria = true; // Se marca como cargado aunque este vacio
        return false;
    }

    lista.usuarios.clear();//Limpiar la lista antes de cargar
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
            
            //  Revisa si el ultimo caracter de la palabra es el salto invisible \r
            if (!perfil.empty() && perfil.back() == '\r') perfil.pop_back();

            Usuario u;
            try {
                u.id = stoi(idStr);
            } catch (...) {
                continue; //Omitir lineas mal tipeadas
            }
            u.nombre = nombre;
            u.username = username;
            u.password = password;
            u.perfil = perfil;

            lista.usuarios.push_back(u);// Agregar usuario a la lista
        }
    }
    file.close();
    lista.cargadoEnMemoria = true;
    return true;
}

// Guardar un usuario en el archivo y en memoria
bool guardarUsuario(ListaUsuarios& lista, const Usuario& nuevoUsuario, const string& rutaArchivo) {
    // Agregar a la lista en memoria
    lista.usuarios.push_back(nuevoUsuario);

    // Agregar registro al final del archivo
    ofstream file(rutaArchivo, ios::app);
    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir el archivo para guardar el usuario." << endl;
        return false;
    }

    file << nuevoUsuario.id << ";"
         << nuevoUsuario.nombre << ";"
         << nuevoUsuario.username << ";"
         << nuevoUsuario.password << ";"
         << nuevoUsuario.perfil << "\n";
    
    file.close();
    return true;
}

void listarUsuarios(ListaUsuarios& lista, const string& rutaArchivo) {
    // Si los datos no están cargados en memoria, se leen del archivo
    if (!lista.cargadoEnMemoria) {
        cargarUsuariosDesdeArchivo(lista, rutaArchivo);
    }

    cout << "\n------------------------------------------------------------\n";
    cout << "                   LISTA DE USUARIOS                        \n";
    cout << "------------------------------------------------------------\n";
    cout << left << setw(6) << "Id"//setw establece el ancho de la columna
         << setw(20) << "Nombre"
         << setw(15) << "Username"
         << setw(15) << "Perfil" << "\n";
    cout << "------------------------------------------------------------\n";
    // Muestra en pantalla los usuarios registrados y sus perfiles
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

bool eliminarUsuarioPorId(ListaUsuarios& lista, int idEliminar, const string& rutaArchivo, bool confirmarAlertaAdmin) {
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

    // Eliminar de memoria
    lista.usuarios.erase(it);

    // Sobrescribir archivo con la lista actualizada
    ofstream file(rutaArchivo, ios::trunc);
    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir el archivo para actualizar la lista de usuarios." << endl;
        return false;
    }
    // Guardar todos los usuarios restantes en el archivo
    for (const auto& u : lista.usuarios) { // Guardar cada usuario en el archivo
        file << u.id << ";"
             << u.nombre << ";"
             << u.username << ";"
             << u.password << ";"
             << u.perfil << "\n";
    }
    file.close();
    return true;
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
        if (u.username == username) return true;
    }
    return false;
}
