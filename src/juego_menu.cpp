// ============================================================================
// ENTREGA 3 - PROBLEMA 2: opcion "3) Juego" del MENU PRINCIPAL
// ----------------------------------------------------------------------------
// Desde aqui se ejecutan, mediante llamadas a sistema, los programas del juego:
//   - juego_server.exe  (GAME_SERVER_EXEC): servidor de la partida
//   - juego_cliente.exe (GAME_CLIENT_EXEC): cliente, juega con el username
// ============================================================================

#include "../include/juego.h"
#include "../include/env.h"
#include "../include/externo.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

using namespace std;

static string recortar(const string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

static string comandoServidor(const string& ejecutable) {
#if defined(_WIN32)
    // Abre el servidor en una ventana de consola nueva.
    return "start \"Servidor - Carrera de Equipos\" \"" + ejecutable + "\"";
#else
    // Ejecuta el servidor en segundo plano (su salida queda en el archivo de log).
    string ruta = ejecutable.find('/') == string::npos ? "./" + ejecutable : ejecutable;
    return "'" + ruta + "' > /dev/null 2>&1 &";
#endif
}

void pantallaJuego(const string& username) {
    int op = -1;
    while (op != 0) {
        cout << "\n============================================================\n";
        cout << "            JUEGO MULTIPLAYER - CARRERA DE EQUIPOS          \n";
        cout << "============================================================\n";
        cout << "1) Unirse a una partida (jugar como '" << username << "')\n";
        cout << "2) Iniciar el servidor del juego\n";
        cout << "0) Volver al menu principal\n";
        cout << "\nOpcion : ";
        string linea;
        if (!getline(cin, linea)) return;
        linea = recortar(linea);
        if (linea == "0") op = 0;
        else if (linea == "1") {
            // Llamada a sistema al cliente del juego (Entrega 3 - Problema 2).
            llamarPrograma("GAME_CLIENT_EXEC", "Entrega 3 - Cliente del juego", {username});
        } else if (linea == "2") {
            string ejecutable = recortar(obtenerVariableEntorno("GAME_SERVER_EXEC"));
            if (ejecutable.empty()) {
                cout << "\nError: la variable de entorno GAME_SERVER_EXEC no esta definida.\n";
                continue;
            }
            ConfigJuego cfg;
            string error;
            if (!cargarConfigJuego(cfg, error)) {
                cout << "\nError de configuracion del juego: " << error << "\n";
                continue;
            }
            cout << flush;
            error_code ec;
            if (!filesystem::exists(ejecutable, ec)) {
                cout << "\nError: no se encontro el programa '" << ejecutable
                     << "'. Compile el proyecto (make) y revise GAME_SERVER_EXEC en el .env.\n";
                continue;
            }
            cout << "\n>> Llamada a sistema: " << ejecutable << " (Entrega 3 - Servidor del juego)\n";
            int r = system(comandoServidor(ejecutable).c_str());
            if (r != 0) {
                cout << "\nNo se pudo iniciar el servidor '" << ejecutable << "' (codigo " << r << ").\n";
            } else {
                cout << "\nServidor iniciado en segundo plano (puerto " << cfg.conexion.puerto
                     << "). Eventos en: " << cfg.archivoLog << "\n";
                cout << "Ahora elija la opcion 1 para unirse a la partida.\n";
            }
        } else {
            cout << "Opcion invalida.\n";
        }
    }
}
