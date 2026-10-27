// ============================================================================
// ENTREGA 3 - PROBLEMA 2: cliente independiente del juego
// Uso:  juego_cliente.exe [nombre_jugador]
// Permite abrir varios jugadores en distintas terminales (o equipos de la red)
// sin necesidad de iniciar sesion en sistope.
// ============================================================================

#include "../include/juego.h"
#include "../include/env.h"

#include <iostream>
#include <string>

using namespace std;

int main(int argc, char* argv[]) {
    // Permite lanzar el cliente desde otra carpeta (busca .env junto al ejecutable).
    string carpetaPrograma;
    if (ubicarCarpetaDelPrograma(argc > 0 ? argv[0] : nullptr, carpetaPrograma))
        cout << "Nota: se usa la carpeta del programa: " << carpetaPrograma << "\n";
    cargarVariablesEntorno(".env");

    string nombre;
    if (argc >= 2) {
        nombre = argv[1];
    } else {
        cout << "Nombre del jugador: ";
        if (!getline(cin, nombre)) return 1;
    }
    return ejecutarClienteJuego(nombre);
}
