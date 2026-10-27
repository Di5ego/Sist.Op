#ifndef JUEGO_H
#define JUEGO_H

// ============================================================================
// ENTREGA 3 - PROBLEMA 2: Juego multiplayer "Carrera de Equipos" mediante sockets
// ----------------------------------------------------------------------------
// Arquitectura:  clientes -> sockets TCP -> servidor -> estado compartido
//
//   juego_server.exe   servidor: acepta conexiones, mantiene el estado de la
//                      partida (equipos, posiciones, turnos) y lo difunde.
//   sistope.exe (op 3) cliente integrado en el menu (juega con su username).
//   juego_cliente.exe  cliente independiente (juego_cliente.exe <nombre>).
//
// Todas las configuraciones se obtienen de variables de entorno (ver README).
// ============================================================================

#include <string>

// Configuracion de la conexion (la usan cliente y servidor).
struct ConfigConexion {
    std::string ipServidor;   // GAME_SERVER_IP
    int puerto = 0;           // GAME_PORT
};

// Configuracion completa del servidor y de las reglas del juego.
struct ConfigJuego {
    ConfigConexion conexion;
    std::string archivoLog;        // GAME_LOG_FILE
    int tamanoTablero = 0;         // GAME_BOARD_SIZE           (X posiciones)
    int carasDado = 0;             // GAME_DICE_FACES           (dado de R caras)
    int minJugadores = 0;          // GAME_MIN_PLAYERS
    int maxJugadores = 0;          // GAME_MAX_PLAYERS
    int minEquipos = 0;            // GAME_MIN_TEAMS
    int maxEquipos = 0;            // GAME_MAX_TEAMS
    int minJugadoresEquipo = 0;    // GAME_MIN_PLAYERS_PER_TEAM
    int maxJugadoresEquipo = 0;    // GAME_MAX_PLAYERS_PER_TEAM
    int segundosTurno = 0;         // GAME_TURN_TIMEOUT (0 = sin limite)
};

// Lee y valida GAME_SERVER_IP y GAME_PORT.
bool cargarConfigConexion(ConfigConexion& cfg, std::string& error);

// Lee y valida todas las variables del servidor (incluye las de conexion).
bool cargarConfigJuego(ConfigJuego& cfg, std::string& error);

// Valida un nombre de jugador: 1 a 20 caracteres [A-Za-z0-9_.-].
bool nombreJugadorValido(const std::string& nombre);

// Cliente del juego: se conecta al servidor y juega con el nombre indicado.
// Devuelve 0 si la sesion termino normalmente, distinto de 0 si hubo error.
int ejecutarClienteJuego(const std::string& nombreJugador);

// Pantalla de la opcion "3) Juego" del menu principal.
void pantallaJuego(const std::string& username);

#endif // JUEGO_H
