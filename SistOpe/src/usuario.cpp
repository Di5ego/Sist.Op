#include "../include/usuario.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip> 
#include <algorithm> 

using namespace std;

bool cargarUsuariosDesdeArchivo(ListaUsuarios& lista, const string& rutaArchivo) {
    // Abre el archivo ubicado en la ruta indicada.
    ifstream file(rutaArchivo);
    // Verifica si el archivo se pudo abrir correctamente.
    if (!file.is_open()) {
        // Muestra un mensaje de advertencia indicando que no se pudo abrir el archivo.
        cerr << "Advertencia: No se pudo abrir el archivo de usuarios: "
             << rutaArchivo << endl;

        // Marca la lista como cargada en memoria, aunque esté vacía.
        lista.cargadoEnMemoria = true;

        // Retorna false para indicar que ocurrió un problema al abrir el archivo.
        return false;
    }

    // Limpia cualquier usuario que ya estuviera almacenado en la lista.
    // Esto evita duplicar usuarios si la función se ejecuta nuevamente.
    lista.usuarios.clear();

    // Variable que almacenará cada línea leída del archivo.
    string line;

    // Lee el archivo línea por línea hasta llegar al final.
    while (getline(file, line)) {

        // Si la línea está vacía, se ignora y se pasa a la siguiente.
        if (line.empty()) continue;
        // Crea un stringstream para poder separar los datos de la línea.
        stringstream ss(line);

        // Variables donde se almacenarán los diferentes datos del usuario.
        string idStr, nombre, username, password, perfil;
        // Lee cada dato de la línea utilizando ';' como separador.
        // Se espera que la línea tenga el siguiente formato:
        // ID;Nombre;Username;Password;Perfil
        if (getline(ss, idStr, ';') &&
            getline(ss, nombre, ';') &&
            getline(ss, username, ';') &&
            getline(ss, password, ';') &&
            getline(ss, perfil)) {

            // Revisa si el último carácter del perfil es '\r'
            // Esto puede aparecer cuando el archivo utiliza formato Windows (CRLF)
            if (!perfil.empty() && perfil.back() == '\r')
                perfil.pop_back(); // Elimina el carácter '\r'

            // Crea una nueva variable de tipo Usuario
            Usuario u;

            // Intenta convertir el ID, que está almacenado como texto a un número entero
            try {
                u.id = stoi(idStr);
            }
            catch (...) {
                // Si el ID no es válido, se ignora esa línea del archivo
                continue;
            }

            // Guarda el nombre leído en el objeto Usuario
            u.nombre = nombre;

            // Guarda el nombre de usuario
            u.username = username;

            // Guarda la contraseña
            u.password = password;

            // Guarda el perfil o tipo de usuario
            u.perfil = perfil;

            // Agrega el usuario creado al final de la lista
            lista.usuarios.push_back(u);
        }
    }

    // Cierra el archivo después de terminar la lectura.
    file.close();

    // Marca la lista como cargada en memoria.
    lista.cargadoEnMemoria = true;

    // Retorna true para indicar que el archivo fue procesado correctamente.
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

    // Verifica si la lista de usuarios ya fue cargada en memoria
    // Si no ha sido cargada, se carga desde el archivo indicado
    if (!lista.cargadoEnMemoria) {
        cargarUsuariosDesdeArchivo(lista, rutaArchivo);
    }

    // Busca dentro de la lista un usuario cuyo ID coincida con el ID
    // que se desea eliminar.
    auto it = find_if(
        lista.usuarios.begin(),              // Inicio de la lista.
        lista.usuarios.end(),                // Fin de la lista.
        [idEliminar](const Usuario& u) {     // Función que revisa cada usuario.
            return u.id == idEliminar;       // Comprueba si el ID coincide.
        }
    );

    // Si el iterador llegó al final de la lista significa que
    // no se encontró ningún usuario con ese ID.
    if (it == lista.usuarios.end()) {

        // Muestra un mensaje indicando que el usuario no existe.
        cout << "Error: No se encontró ningún usuario con el ID "
             << idEliminar << ".\n";

        // Retorna false para indicar que no se pudo realizar la eliminación.
        return false;
    }

    // Elimina de la memoria el usuario encontrado.
    // 'it' apunta directamente al usuario que se encontró anteriormente.
    lista.usuarios.erase(it);

    // Abre el archivo para sobrescribirlo completamente.
    // ios::trunc elimina el contenido anterior del archivo.
    ofstream file(rutaArchivo, ios::trunc);

    // Verifica si el archivo se pudo abrir correctamente.
    if (!file.is_open()) {

        // Muestra un mensaje de error indicando que no se pudo abrir el archivo
        cerr << "Error: No se pudo abrir el archivo para actualizar "
             << "la lista de usuarios." << endl;

        // Retorna false porque no se pudo guardar el cambio en el archivo.
        return false;
    }

    // Recorre todos los usuarios que quedaron despues de la eliminacion
    for (const auto& u : lista.usuarios) {

        // Escribe cada usuario en el archivo, separando los campos con ';'
        file << u.id << ";"
             << u.nombre << ";"
             << u.username << ";"
             << u.password << ";"
             << u.perfil << "\n";
    }

    // Cierra el archivo después de terminar de guardar los usuarios.
    file.close();

    // Retorna true para indicar que el usuario fue eliminado
    // y la lista actualizada fue guardada correctamente.
    return true;
}


// Elimina todos los usuarios que tengan asignado el perfil especificado (eliminación en cascada)
int eliminarUsuariosPorPerfil(ListaUsuarios& lista, const string& nombrePerfil, const string& rutaArchivo) {
    if (!lista.cargadoEnMemoria) {
        cargarUsuariosDesdeArchivo(lista, rutaArchivo);
    }

    size_t tamInicial = lista.usuarios.size();
    lista.usuarios.erase(
        remove_if(lista.usuarios.begin(), lista.usuarios.end(), [&nombrePerfil](const Usuario& u) {
            return u.perfil == nombrePerfil;
        }),
        lista.usuarios.end()
    );

    int eliminados = static_cast<int>(tamInicial - lista.usuarios.size());
    if (eliminados > 0) {
        ofstream file(rutaArchivo, ios::trunc);
        if (file.is_open()) {
            for (const auto& u : lista.usuarios) {
                file << u.id << ";"
                     << u.nombre << ";"
                     << u.username << ";"
                     << u.password << ";"
                     << u.perfil << "\n";
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
        if (u.username == username) return true;
    }
    return false;
}


