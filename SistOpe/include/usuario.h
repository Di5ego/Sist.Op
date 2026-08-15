#ifndef USUARIO_H
#define USUARIO_H

#include <string>
#include <vector>
using namespace std;

//Struct usuario  [int, string, string, string, string ]
struct Usuario { 
    int id;
    string nombre;
    string username;
    string password;
    string perfil;
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
bool existeUsuarioId(const ListaUsuarios& lista, int id);
bool existeUsername(const ListaUsuarios& lista, const string& username);

#endif // USUARIO_H
