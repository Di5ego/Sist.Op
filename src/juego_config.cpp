// ============================================================================
// ENTREGA 3 - PROBLEMA 2: lectura y validacion de las variables de entorno del
// juego. Ningun valor de configuracion esta escrito en el codigo: si una
// variable falta o es invalida se informa con un mensaje claro y no se inicia.
// ============================================================================

#include "../include/juego.h"
#include "../include/env.h"

#include <cctype>
#include <string>

using namespace std;

static string recortar(const string& s) {
    size_t a = s.find_first_not_of(" \t\r\n\"'");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n\"'");
    return s.substr(a, b - a + 1);
}

// Lee la variable 'nombre' como entero dentro de [minimo, maximo].
static bool leerEntero(const string& nombre, int minimo, int maximo, int& salida, string& error) {
    string valor = recortar(obtenerVariableEntorno(nombre));
    if (valor.empty()) {
        error = "La variable de entorno " + nombre + " no esta definida.";
        return false;
    }
    bool soloDigitos = valor.size() <= 9;
    for (char c : valor) if (!isdigit(static_cast<unsigned char>(c))) soloDigitos = false;
    if (!soloDigitos) {
        error = nombre + "='" + valor + "' no es un numero entero valido.";
        return false;
    }
    int n = stoi(valor);
    if (n < minimo || n > maximo) {
        error = nombre + "=" + valor + " esta fuera de rango (debe estar entre " + to_string(minimo)
                + " y " + to_string(maximo) + ").";
        return false;
    }
    salida = n;
    return true;
}

bool cargarConfigConexion(ConfigConexion& cfg, string& error) {
    cfg.ipServidor = recortar(obtenerVariableEntorno("GAME_SERVER_IP"));
    if (cfg.ipServidor.empty()) {
        error = "La variable de entorno GAME_SERVER_IP no esta definida (ej: GAME_SERVER_IP=127.0.0.1).";
        return false;
    }
    if (cfg.ipServidor.find_first_of(" \t;") != string::npos) {
        error = "GAME_SERVER_IP='" + cfg.ipServidor + "' no es una direccion valida.";
        return false;
    }
    return leerEntero("GAME_PORT", 1, 65535, cfg.puerto, error);
}

//--------------------------------------------------------------------------------------------------------------------------

// 2.28) CONFIGURACION DEL JUEGO
// Todas las reglas del juego salen de variables de entorno (.env), no del codigo. Si una variable falta o es invalida, 
//se informa y el servidor NO arranca.
bool cargarConfigJuego(ConfigJuego& cfg, string& error) {
    if (!cargarConfigConexion(cfg.conexion, error)) return false;

    //Punto 2.11 el nombre/ruta del log viene de GAME_LOG_FILE. El nombre del archivo no esta escrito en el codigo.
    cfg.archivoLog = recortar(obtenerVariableEntorno("GAME_LOG_FILE"));
    if (cfg.archivoLog.empty()) {
        error = "La variable de entorno GAME_LOG_FILE no esta definida (ej: GAME_LOG_FILE=juego.log).";
        return false;
    }

    // Punto 2.4 y 2.6: cantidades de equipos y jugadores, tablero y dado.
    // Cada valor se valida dentro de un rango (ej: GAME_BOARD_SIZE entre 5 y 1000).
    // leerEntero rechaza valores vacios, con letras o fuera de rango, con un mensaje claro.
    if (!leerEntero("GAME_BOARD_SIZE", 5, 1000, cfg.tamanoTablero, error)) return false;
    if (!leerEntero("GAME_DICE_FACES", 2, 100, cfg.carasDado, error)) return false;
    if (!leerEntero("GAME_MIN_TEAMS", 2, 26, cfg.minEquipos, error)) return false;
    if (!leerEntero("GAME_MAX_TEAMS", 2, 26, cfg.maxEquipos, error)) return false;
    if (!leerEntero("GAME_MIN_PLAYERS_PER_TEAM", 1, 50, cfg.minJugadoresEquipo, error)) return false;
    if (!leerEntero("GAME_MAX_PLAYERS_PER_TEAM", 1, 50, cfg.maxJugadoresEquipo, error)) return false;
    if (!leerEntero("GAME_MIN_PLAYERS", 2, 100, cfg.minJugadores, error)) return false;
    if (!leerEntero("GAME_MAX_PLAYERS", 2, 100, cfg.maxJugadores, error)) return false;
    if (!leerEntero("GAME_TURN_TIMEOUT", 0, 3600, cfg.segundosTurno, error)) return false;
    

    // Coherencia entre variables: no basta con que cada valor sea valido por separado.
    // Ej: el minimo de jugadores debe alcanzar para cubrir los equipos minimos
    // (GAME_MIN_PLAYERS >= GAME_MIN_TEAMS x GAME_MIN_PLAYERS_PER_TEAM).
    // Si no se cumple, la partida nunca podria comenzar.
    if (cfg.minEquipos > cfg.maxEquipos) {
        error = "GAME_MIN_TEAMS (" + to_string(cfg.minEquipos) + ") no puede ser mayor que GAME_MAX_TEAMS ("
                + to_string(cfg.maxEquipos) + ").";
        return false;
    }
    if (cfg.minJugadoresEquipo > cfg.maxJugadoresEquipo) {
        error = "GAME_MIN_PLAYERS_PER_TEAM (" + to_string(cfg.minJugadoresEquipo)
                + ") no puede ser mayor que GAME_MAX_PLAYERS_PER_TEAM (" + to_string(cfg.maxJugadoresEquipo) + ").";
        return false;
    }
    if (cfg.minJugadores > cfg.maxJugadores) {
        error = "GAME_MIN_PLAYERS (" + to_string(cfg.minJugadores) + ") no puede ser mayor que GAME_MAX_PLAYERS ("
                + to_string(cfg.maxJugadores) + ").";
        return false;
    }
    if (cfg.minJugadores < cfg.minEquipos * cfg.minJugadoresEquipo) {
        error = "GAME_MIN_PLAYERS (" + to_string(cfg.minJugadores) + ") debe ser al menos GAME_MIN_TEAMS x "
                "GAME_MIN_PLAYERS_PER_TEAM = " + to_string(cfg.minEquipos * cfg.minJugadoresEquipo) + ".";
        return false;
    }
    if (cfg.maxJugadores > cfg.maxEquipos * cfg.maxJugadoresEquipo) {
        error = "GAME_MAX_PLAYERS (" + to_string(cfg.maxJugadores) + ") no puede superar GAME_MAX_TEAMS x "
                "GAME_MAX_PLAYERS_PER_TEAM = " + to_string(cfg.maxEquipos * cfg.maxJugadoresEquipo) + ".";
        return false;
    }
    return true;
}

bool nombreJugadorValido(const string& nombre) {
    if (nombre.empty() || nombre.size() > 20) return false;
    for (char c : nombre) {
        unsigned char u = static_cast<unsigned char>(c);
        if (!isalnum(u) && c != '_' && c != '-' && c != '.') return false;
    }
    return true;
}
