#ifndef USUARIO_H
#define USUARIO_H

#include <string>
#include <vector>
#include <cstring>

using namespace std;

// Struct Usuario definido con campos de longitud fija (POD / Trivially Copyable)
// para permitir lectura y escritura directa en un solo bloque binario con read() y write()
// Referencia de la pauta: https://stackoverflow.com/q/74452768
struct Usuario { 
    int id;
    char nombre[50];
    char username[30];
    char password[30];
    char perfil[30];

    // Constructor por defecto
    Usuario() : id(0) {
        nombre[0] = '\0';
        username[0] = '\0';
        password[0] = '\0';
        perfil[0] = '\0';
    }

    // Constructor con parametros
    Usuario(int _id, const string& _nombre, const string& _username, const string& _password, const string& _perfil) {
        id = _id;
        setNombre(_nombre);
        setUsername(_username);
        setPassword(_password);
        setPerfil(_perfil);
    }

    // Metodos auxiliares para asignacion segura
    void setNombre(const string& s) {
        strncpy(nombre, s.c_str(), sizeof(nombre) - 1);
        nombre[sizeof(nombre) - 1] = '\0';
    }

    void setUsername(const string& s) {
        strncpy(username, s.c_str(), sizeof(username) - 1);
        username[sizeof(username) - 1] = '\0';
    }

    void setPassword(const string& s) {
        strncpy(password, s.c_str(), sizeof(password) - 1);
        password[sizeof(password) - 1] = '\0';
    }

    void setPerfil(const string& s) {
        strncpy(perfil, s.c_str(), sizeof(perfil) - 1);
        perfil[sizeof(perfil) - 1] = '\0';
    }
};

// Struct lista de usuarios para mantenerlos en memoria
struct ListaUsuarios {
    vector<Usuario> usuarios; 
    bool cargadoEnMemoria = false; // Indica si los usuarios han sido cargados desde el archivo
};

// Funciones de gestion de usuarios
bool cargarUsuariosDesdeArchivo(ListaUsuarios& lista, const string& rutaArchivo);
bool guardarUsuario(ListaUsuarios& lista, const Usuario& nuevoUsuario, const string& rutaArchivo);
void listarUsuarios(ListaUsuarios& lista, const string& rutaArchivo);
bool eliminarUsuarioPorId(ListaUsuarios& lista, int idEliminar, const string& rutaArchivo, bool confirmarAlertaAdmin);
int eliminarUsuariosPorPerfil(ListaUsuarios& lista, const string& nombrePerfil, const string& rutaArchivo);
bool existeUsuarioId(const ListaUsuarios& lista, int id);
bool existeUsername(const ListaUsuarios& lista, const string& username);

#endif // USUARIO_H
