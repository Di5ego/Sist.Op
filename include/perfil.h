#ifndef PERFIL_H
#define PERFIL_H

#include <string>
#include <vector>
#include <cstring>
#include <algorithm>

using namespace std;

// Struct Perfil definido con campos de longitud fija (POD / Trivially Copyable)
// para permitir lectura y escritura directa en un solo bloque binario con read() y write()
// Referencia requerida en la pauta: https://stackoverflow.com/q/74452768
struct Perfil {
    char nombre[30];
    int cantidadOpciones;
    int opciones[20]; // Array de enteros fijo con capacidad para hasta 20 opciones de menu

    // Constructor por defecto
    Perfil() : cantidadOpciones(0) {
        nombre[0] = '\0';
        for (int i = 0; i < 20; ++i) opciones[i] = 0;
    }

    // Constructor con parametros
    Perfil(const string& _nombre, const vector<int>& _opciones) : cantidadOpciones(0) {
        setNombre(_nombre);
        setOpciones(_opciones);
    }

    // Metodos auxiliares para asignacion segura
    void setNombre(const string& s) {
        strncpy(nombre, s.c_str(), sizeof(nombre) - 1);
        nombre[sizeof(nombre) - 1] = '\0';
    }

    void setOpciones(const vector<int>& ops) {
        cantidadOpciones = std::min(static_cast<int>(ops.size()), 20);
        for (int i = 0; i < cantidadOpciones; ++i) {
            opciones[i] = ops[i];
        }
    }

    void agregarOpcion(int op) {
        if (cantidadOpciones < 20) {
            opciones[cantidadOpciones++] = op;
        }
    }

    void limpiarOpciones() {
        cantidadOpciones = 0;
    }

    vector<int> getOpcionesVector() const {
        vector<int> v;
        for (int i = 0; i < cantidadOpciones; ++i) {
            v.push_back(opciones[i]);
        }
        return v;
    }

    bool tieneOpcion(int op) const {
        for (int i = 0; i < cantidadOpciones; ++i) {
            if (opciones[i] == op) return true;
        }
        return false;
    }
};

// Struct lista de perfiles para mantener perfiles en memoria
struct ListaPerfiles {
    vector<Perfil> perfiles; 
    bool cargadoEnMemoria = false; // Indica si los perfiles han sido cargados desde el archivo
};

// Funciones de gestion de perfiles
bool cargarPerfilesDesdeArchivo(ListaPerfiles& lista, const string& rutaArchivo); 
bool guardarPerfil(ListaPerfiles& lista, const Perfil& nuevoPerfil, const string& rutaArchivo);
void listarPerfiles(ListaPerfiles& lista, const string& rutaArchivo);
bool eliminarPerfilPorNombre(ListaPerfiles& lista, const string& nombrePerfil, const string& rutaArchivo);
bool existePerfil(const ListaPerfiles& lista, const string& nombrePerfil);
bool perfilPermiteOpcion(const Perfil& perfil, int opcion);

#endif // PERFIL_H
