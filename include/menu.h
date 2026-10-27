#ifndef MENU_H
#define MENU_H

#include "usuario.h"
#include "perfil.h"
#include "cli.h"
#include <string>

using namespace std;

// MENU PRINCIPAL (Entrega 2, ampliado en la Entrega 3).
// Las opciones 1, 2, 3 y 8 ejecutan los programas de cada entrega mediante
// llamadas a sistema (ver externo.h).
void mostrarMenuPrincipalEntrega2(SesionUsuario& sesion, ListaUsuarios& listaUsuarios, ListaPerfiles& listaPerfiles, const string& rutaUsuarios, const string& rutaPerfiles);

#endif // MENU_H
