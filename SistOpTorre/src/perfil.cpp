#include "../include/perfil.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace std;

bool cargarPerfilesDesdeArchivo(ListaPerfiles& lista, const string& rutaArchivo) {
    ifstream file(rutaArchivo);
    if (!file.is_open()) {
        cerr << "Advertencia: No se pudo abrir el archivo de perfiles: " << rutaArchivo << endl;
        lista.cargadoEnMemoria = true;
        return false;
    }

    lista.perfiles.clear();// Limpiar la lista antes de cargar
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string nombre, opcionesStr;

        if (getline(ss, nombre, ';') && getline(ss, opcionesStr)) {
            //  Revisa si el ultimo caracter de la palabra es el salto invisible \r
            if (!opcionesStr.empty() && opcionesStr.back() == '\r') opcionesStr.pop_back();

            Perfil p;
            p.nombre = nombre;
            //Tokens: caracteres separados por comas que representan IDs de opciones
            stringstream ssOps(opcionesStr);
            string opToken;
            while (getline(ssOps, opToken, ',')) { 
                try {
                    p.opciones.push_back(stoi(opToken)); 
                } catch (...) {
                    //ignora caracteres invalidos
                }
            }

            lista.perfiles.push_back(p);// Agregar perfil a la lista
        }
    }
    file.close();
    lista.cargadoEnMemoria = true;
    return true;
}

// Guardar un perfil en el archivo y en memoria
bool guardarPerfil(ListaPerfiles& lista, const Perfil& nuevoPerfil, const string& rutaArchivo) {
    lista.perfiles.push_back(nuevoPerfil);// Agregar a la lista en memoria

    // Agregar registro al final del archivo 
    ofstream file(rutaArchivo, ios::app);
    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir el archivo para guardar el perfil." << endl;
        return false;
    }

    file << nuevoPerfil.nombre << ";";
    for (size_t i = 0; i < nuevoPerfil.opciones.size(); ++i) {
        file << nuevoPerfil.opciones[i];
        if (i + 1 < nuevoPerfil.opciones.size()) {
            file << ",";
        }
    }
    file << "\n";
    file.close();
    return true;
}


void listarPerfiles(ListaPerfiles& lista, const string& rutaArchivo) {
    if (!lista.cargadoEnMemoria) {
        cargarPerfilesDesdeArchivo(lista, rutaArchivo);
    }

    cout << "\n------------------------------------------------------------\n";
    cout << "                   LISTA DE PERFILES                        \n";
    cout << "------------------------------------------------------------\n";
    cout << left << setw(20) << "Nombre Perfil"
         << "Opciones Permitiadas (IDs)\n";
    cout << "------------------------------------------------------------\n";
    // Muestra en pantalla los perfiles registrados y sus opciones
    if (lista.perfiles.empty()) {
        cout << "(No hay perfiles registrados)\n";
    } else {
        for (const auto& p : lista.perfiles) {
            cout << left << setw(20) << p.nombre;
            string opsStr = "";
            for (size_t i = 0; i < p.opciones.size(); ++i) {
                opsStr += to_string(p.opciones[i]);
                if (i + 1 < p.opciones.size()) opsStr += ", ";
            }
            cout << opsStr << "\n";
        }
    }
    cout << "------------------------------------------------------------\n";
}

bool eliminarPerfilPorNombre(ListaPerfiles& lista, const string& nombrePerfil, const string& rutaArchivo) {
    if (!lista.cargadoEnMemoria) {
        cargarPerfilesDesdeArchivo(lista, rutaArchivo);
    }

    auto it = find_if(lista.perfiles.begin(), lista.perfiles.end(), [&nombrePerfil](const Perfil& p) {
        return p.nombre == nombrePerfil;
    });

    if (it == lista.perfiles.end()) {
        cout << "Error: No se encontró ningún perfil llamado '" << nombrePerfil << "'.\n";
        return false;
    }

    // Eliminar de memoria
    lista.perfiles.erase(it);

    // Sobrescribir archivo con la lista actualizada
    ofstream file(rutaArchivo, ios::trunc);
    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir el archivo para actualizar la lista de perfiles" << endl;
        return false;
    }
    // Guarda los perfiles y sus opciones en el archivo
    for (const auto& p : lista.perfiles) { 
        file << p.nombre << ";";
        for (size_t i = 0; i < p.opciones.size(); ++i) {
            file << p.opciones[i];
            if (i + 1 < p.opciones.size()) file << ",";
        }
        file << "\n";
    }
    file.close();
    return true;
}

//Verifica si un perfil con el nombre dado ya existe en la lista
bool existePerfil(const ListaPerfiles& lista, const string& nombrePerfil) {
    for (const auto& p : lista.perfiles) {
        if (p.nombre == nombrePerfil) return true;
    }
    return false;
}
