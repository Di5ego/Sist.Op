#ifndef ADMIN_USUARIOS_H
#define ADMIN_USUARIOS_H

// ENTREGA 1 - Administracion de Usuarios y Perfiles (programa admin_usuarios.exe)

#include "usuario.h"
#include "perfil.h"
#include <string>

// Menu del programa de la Entrega 1 (Gestion de Usuarios / Gestion de Perfiles).
void ejecutarAdministradorUsuariosYPerfiles(ListaUsuarios& listaUsuarios, ListaPerfiles& listaPerfiles,
                                            const std::string& rutaUsuarios, const std::string& rutaPerfiles);

// Modulos de la Entrega 1
void menuGestionUsuarios(ListaUsuarios& listaUsuarios, ListaPerfiles& listaPerfiles,
                         const std::string& rutaUsuarios, const std::string& rutaPerfiles);
void menuGestionPerfiles(ListaPerfiles& listaPerfiles, ListaUsuarios& listaUsuarios,
                         const std::string& rutaPerfiles, const std::string& rutaUsuarios);

#endif // ADMIN_USUARIOS_H
