#ifndef MENU_H
#define MENU_H

#include "usuario.h"
#include "perfil.h"
#include <string>
using namespace std;

//Menu principal
void mostrarMenuPrincipal(ListaUsuarios& listaUsuarios, ListaPerfiles& listaPerfiles, const string& rutaUsuarios, const string& rutaPerfiles);

//Menu usuarios
void menuGestionUsuarios(ListaUsuarios& listaUsuarios, ListaPerfiles& listaPerfiles, const string& rutaUsuarios, const string& rutaPerfiles);

//Menu perfiles
void menuGestionPerfiles(ListaPerfiles& listaPerfiles, const string& rutaPerfiles);

#endif 
