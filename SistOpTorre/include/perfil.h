#ifndef PERFIL_H
#define PERFIL_H

#include <string>
#include <vector>
using namespace std;

// Struct Perfil  [string, array de enteros ]
struct Perfil {
    string nombre; //nombre del perfil (ADMIN, GENERAL, etc)
    vector<int> opciones; //acciones que puede realizar el perfil
};

// Struct lista de perfiles para mantener perfiles en memoria
struct ListaPerfiles {
    vector<Perfil> perfiles; 
    bool cargadoEnMemoria = false; //Indica si los perfiles han sido cargados desde el archivo
};

// Funciones de gestion de perfiles
bool cargarPerfilesDesdeArchivo(ListaPerfiles& lista, const string& rutaArchivo); 
bool guardarPerfil(ListaPerfiles& lista, const Perfil& nuevoPerfil, const string& rutaArchivo);
void listarPerfiles(ListaPerfiles& lista, const string& rutaArchivo);
bool eliminarPerfilPorNombre(ListaPerfiles& lista, const string& nombrePerfil, const string& rutaArchivo);
bool existePerfil(const ListaPerfiles& lista, const string& nombrePerfil);

#endif // PERFIL_H
