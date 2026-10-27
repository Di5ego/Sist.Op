// ============================================================================
// ENTREGA 3 - PROBLEMA 1: Programa "CREAR INDICE INVERTIDO"
// ----------------------------------------------------------------------------
// Uso:  indice_invertido.exe [username] [perfil]
//
// El MENU PRINCIPAL (sistope.exe) lo ejecuta en su opcion 8 mediante una
// llamada a sistema. La logica (cola, threads y semaforos) esta en
// src/indice_invertido.cpp.
// ============================================================================

#include "../include/indice_invertido.h"
#include "../include/env.h"

#include <iostream>
#include <string>

using namespace std;

// -------------------------------------------------------------------------------------------------------------------------------------------------

int main(int argc, char* argv[]) {
    string carpetaPrograma;
    ubicarCarpetaDelPrograma(argc > 0 ? argv[0] : nullptr, carpetaPrograma);
    // 1.3)EL PROGRAMA DEL INDICE ARRANCA
    // indice_invertido.exe es un programa independiente: puede ejecutarse solo (sin el menú). Carga el .env y entra 
    // a la pantalla de la funcionalidad.
    cargarVariablesEntorno(".env"); //#########################

    if (argc >= 3) {
        cout << "============================================================\n";
        cout << "User: " << argv[1] << " (" << argv[2] << ")\n";
        cout << "============================================================\n";
    }
    pantallaCrearIndiceInvertido(); //#########################
    return 0;
}
