#ifndef MENU_H
#define MENU_H

#include "usuario.h"
#include "perfil.h"
#include "cli.h"
#include <string>

using namespace std;

// Menu principal Entrega 2
void mostrarMenuPrincipalEntrega2(SesionUsuario& sesion, ListaUsuarios& listaUsuarios, ListaPerfiles& listaPerfiles, const string& rutaUsuarios, const string& rutaPerfiles);

// Menus de Entrega 1 (Gestion de usuarios y perfiles)
void menuGestionUsuarios(ListaUsuarios& listaUsuarios, ListaPerfiles& listaPerfiles, const string& rutaUsuarios, const string& rutaPerfiles);
void menuGestionPerfiles(ListaPerfiles& listaPerfiles, ListaUsuarios& listaUsuarios, const string& rutaPerfiles, const string& rutaUsuarios);

#endif // MENU_H
