// ============================================================================
// ENTREGA 3 - PROBLEMA 2: SERVIDOR del juego multiplayer "Carrera de Equipos"
// ----------------------------------------------------------------------------
// Programa independiente:  juego_server.exe
//
// Arquitectura (clientes -> sockets -> servidor -> estado compartido):
//
//   cliente 1 --TCP--> [thread atencion 1] --+
//   cliente 2 --TCP--> [thread atencion 2] ----> ESTADO COMPARTIDO DEL JUEGO
//   cliente N --TCP--> [thread atencion N] --/   (protegido por mtxEstado)
//                      [thread temporizador] -/   equipos, posiciones, turnos
//   [thread principal]: acepta conexiones y realiza el cierre controlado
//
// - Cada cliente es atendido por su propio thread, que lee sus comandos
//   (una linea de texto por mensaje) y los aplica sobre el estado compartido.
// - Toda lectura/modificacion del estado ocurre dentro de la seccion critica
//   protegida por 'mtxEstado', por lo que dos jugadores nunca pueden alterar
//   la partida al mismo tiempo (ej: dos lanzamientos simultaneos).
// - El servidor es la UNICA autoridad: tira los dados, valida turnos y
//   difunde (broadcast) cada cambio a todos los jugadores.
// - Cada evento relevante se registra en el archivo GAME_LOG_FILE.
// ============================================================================

#include "../include/juego.h"
#include "../include/red.h"
#include "../include/env.h"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <csignal>
#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace std;
using Reloj = chrono::steady_clock;

// ----------------------------------------------------------------------------
// Registro de eventos (LOG)
// Formato de cada linea (consistente durante toda la ejecucion):
//     AAAA-MM-DD HH:MM:SS;jugador;equipo;ACCION;detalle
// Un campo que no aplica se registra como '-'. El detalle usa pares
// clave=valor separados por ','.
// ----------------------------------------------------------------------------
class RegistroLog {
public:
    bool abrir(const string& ruta, string& error) {
        archivo.open(ruta, ios::out | ios::app);
        if (!archivo.is_open()) {
            error = "No se pudo abrir el archivo de log '" + ruta + "' (revise GAME_LOG_FILE y los permisos).";
            return false;
        }
        return true;
    }

// -------------------------------------------------------------------------------------------------------------------------------------------------

    //2.23) EL ARCHIVO DE LOG
    void registrar(const string& jugador, const string& equipo, const string& accion, const string& detalle) {

        // SECCIÓN CRÍTICA 4: el log tiene su PROPIO mutex porque lo escriben varios threads. Sin esto, dos líneas 
        //podrían mezclarse en el archivo.
        lock_guard<mutex> guardia(mtx);  

        // FORMATO FIJO de 5 campos, igual durante toda la ejecución: fecha hora ; jugador ; equipo ; ACCION ; detalle
        // Un campo que no aplica se escribe como '-'
        string linea = fechaHora() + ";" + limpiar(jugador) + ";" + limpiar(equipo) + ";" +
                       limpiar(accion) + ";" + limpiar(detalle);
        if (archivo.is_open()) {
            archivo << linea << '\n';
            archivo.flush();
        }
        cout << "[LOG] " << linea << endl;
    }

    void cerrar() {
        lock_guard<mutex> guardia(mtx);  
        if (archivo.is_open()) archivo.close();
    }

private:
    static string limpiar(const string& s) {
        if (s.empty()) return "-";
        string r = s;
        for (char& c : r) if (c == ';' || c == '\n' || c == '\r') c = ',';
        return r;
    }
    static string fechaHora() {
        time_t ahora = time(nullptr);
        tm t;
#if defined(_WIN32)
        localtime_s(&t, &ahora);
#else
        localtime_r(&ahora, &t);
#endif
        char buf[32];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &t);
        return buf;
    }
    mutex mtx;
    ofstream archivo;
};

// ----------------------------------------------------------------------------
// Estado compartido del juego
// ----------------------------------------------------------------------------
enum class Fase { ESPERA, EN_CURSO, TERMINADA };

struct Jugador {
    int id = 0;                       // identificador interno de la conexion
    socket_t sock = SOCKET_INVALIDO;
    string ip;
    string nombre;                    // vacio hasta que se identifica (HOLA)
    bool registrado = false;
    bool conectado = true;
    int equipo = -1;                  // indice en 'equipos' o -1
    bool listo = false;
};

struct Equipo {
    string nombre;                    // EquipoA, EquipoB, ...
    int posicion = 0;
    vector<int> miembros;             // ids de jugadores, en orden de llegada
    size_t siguienteMiembro = 0;      // a quien le toca dentro del equipo
    bool participa = false;           // tiene jugadores al iniciar la partida
    bool eliminado = false;           // se quedo sin jugadores durante la partida
};

class ServidorJuego {
public:
    explicit ServidorJuego(const ConfigJuego& c) : cfg(c), rng(random_device{}() ^ static_cast<unsigned>(time(nullptr))) {
        for (int i = 0; i < cfg.maxEquipos; ++i) {
            Equipo e;
            e.nombre = string("Equipo") + static_cast<char>('A' + i);
            equipos.push_back(e);
        }
    }

    int ejecutar();                    // bucle principal
    static atomic<bool> senalTerminar; // Ctrl+C

private:
    // --- acciones (todas se llaman con mtxEstado tomado) ---
    void procesarComando(const shared_ptr<Jugador>& j, const string& linea, bool& seguir);
    void cmdHola(const shared_ptr<Jugador>& j, const string& arg, bool& seguir);
    void cmdEquipo(const shared_ptr<Jugador>& j, const string& arg);
    void cmdListo(const shared_ptr<Jugador>& j);
    void cmdLanzar(const shared_ptr<Jugador>& j);
    void cmdChat(const shared_ptr<Jugador>& j, const string& texto);
    void enviarEstado(const shared_ptr<Jugador>& j);
    void enviarAyuda(const shared_ptr<Jugador>& j);
    void verificarInicio();
    void difundirEspera();
    void iniciarPartida();
    void siguienteTurno();
    void ejecutarLanzamiento(const shared_ptr<Jugador>& j, bool automatico);
    void terminarPartida(int equipoGanador, const string& jugadorGanador, const string& motivo);
    void manejarDesconexion(const shared_ptr<Jugador>& j, const string& motivo);

    // --- utilidades ---
    void enviar(const shared_ptr<Jugador>& j, const string& msg);
    void difundir(const string& msg);
    shared_ptr<Jugador> buscarJugador(int id);
    string nombreEquipoDe(const shared_ptr<Jugador>& j);
    string describirEquipo(const Equipo& e);
    int jugadoresRegistrados();
    int conexionesAbiertas();
    int equiposActivos();

    // --- threads ---
    void atenderCliente(shared_ptr<Jugador> j);
    void temporizador();

    ConfigJuego cfg;
    RegistroLog log;

// -------------------------------------------------------------------------------------------------------------------------------------------------    
    
    // 2.14) EL ESTADO COMPARTIDO
    // ESTADO COMPARTIDO: jugadores, equipos, posiciones, fase y turno. Un único mutex protege TODO. Cualquier thread que quiera leer 
    // o modificar la partida primero debe tomar este candado. El servidor es la única fuente de verdad: los clientes nunca calculan nada.
    mutex mtxEstado;                          // protege TODO el estado de abajo
    vector<shared_ptr<Jugador>> jugadores;    
    vector<Equipo> equipos;
    Fase fase = Fase::ESPERA;                 // ESPERA, EN_CURSO, TERMINADA
    int idTurno = -1;                         
    int equipoTurno = -1;                     
    int numeroTurno = 0;
    Reloj::time_point limiteTurno;
    mt19937 rng;
    int siguienteId = 1;

    atomic<bool> detener{false};              // la partida termino -> cerrar servidor
    vector<thread> hilosClientes;             // solo los usa el thread principal
};

atomic<bool> ServidorJuego::senalTerminar{false};

static void manejadorSenal(int) { ServidorJuego::senalTerminar = true; }

static string aMayusculas(string s) {
    for (char& c : s) c = static_cast<char>(toupper(static_cast<unsigned char>(c)));
    return s;
}

static string recortar(const string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

// ----------------------------------------------------------------------------
// Utilidades de envio
// ----------------------------------------------------------------------------
void ServidorJuego::enviar(const shared_ptr<Jugador>& j, const string& msg) {
    if (j && j->conectado) redEnviarLinea(j->sock, msg);   // si falla, el thread del cliente detecta la desconexion
}

void ServidorJuego::difundir(const string& msg) {
    for (const auto& j : jugadores)
        if (j->conectado && j->registrado) redEnviarLinea(j->sock, msg);
}

shared_ptr<Jugador> ServidorJuego::buscarJugador(int id) {
    for (const auto& j : jugadores) if (j->id == id) return j;
    return nullptr;
}

string ServidorJuego::nombreEquipoDe(const shared_ptr<Jugador>& j) {
    if (!j || j->equipo < 0) return "-";
    return equipos[j->equipo].nombre;
}

string ServidorJuego::describirEquipo(const Equipo& e) {
    string s;
    for (size_t k = 0; k < e.miembros.size(); ++k) {
        auto m = buscarJugador(e.miembros[k]);
        if (!m) continue;
        if (!s.empty()) s += ", ";
        s += m->nombre;
        if (fase == Fase::ESPERA) s += m->listo ? " (listo)" : " (no listo)";
    }
    return s.empty() ? "(vacio)" : s;
}

int ServidorJuego::jugadoresRegistrados() {
    int n = 0;
    for (const auto& j : jugadores) if (j->conectado && j->registrado) n++;
    return n;
}

int ServidorJuego::conexionesAbiertas() {
    int n = 0;
    for (const auto& j : jugadores) if (j->conectado) n++;
    return n;
}

int ServidorJuego::equiposActivos() {
    int n = 0;
    for (const auto& e : equipos) if (e.participa && !e.eliminado) n++;
    return n;
}

// ----------------------------------------------------------------------------
// Comandos del protocolo (cliente -> servidor)
//   HOLA <nombre> | EQUIPO <letra> | LISTO | LANZAR | CHAT <texto> |
//   ESTADO | AYUDA | SALIR
// ----------------------------------------------------------------------------
void ServidorJuego::procesarComando(const shared_ptr<Jugador>& j, const string& lineaRecibida, bool& seguir) {
    string linea = recortar(lineaRecibida);
    if (linea.empty()) return;
    size_t esp = linea.find(' ');
    string cmd = aMayusculas(esp == string::npos ? linea : linea.substr(0, esp));
    string arg = esp == string::npos ? "" : recortar(linea.substr(esp + 1));

// -------------------------------------------------------------------------------------------------------------------------------------------------

    //2.16) EL PROTOCOLO DE COMANDOS
// PROTOCOLO: cada línea es un comando. Primero el jugador debe identificarse con HOLA; después puede usar el resto de comandos.
    if (!j->registrado) {
        if (cmd == "HOLA") cmdHola(j, arg, seguir);
        else if (cmd == "SALIR") seguir = false;
        else enviar(j, "ERROR Primero debe identificarse con: HOLA <nombre>");
        return;
    }

    if (cmd == "EQUIPO") cmdEquipo(j, arg);
    else if (cmd == "LISTO") cmdListo(j);
    else if (cmd == "LANZAR") cmdLanzar(j);
    else if (cmd == "CHAT") cmdChat(j, arg);
    else if (cmd == "ESTADO") enviarEstado(j);
    else if (cmd == "AYUDA") enviarAyuda(j);
    else if (cmd == "SALIR") seguir = false;
    else if (cmd == "HOLA") enviar(j, "ERROR Ya esta identificado como " + j->nombre + ".");
    else {
        enviar(j, "ERROR Comando desconocido '" + cmd + "'. Escriba AYUDA para ver los comandos.");
        log.registrar(j->nombre, nombreEquipoDe(j), "ACCION_RECHAZADA", "comando=" + cmd + ",motivo=desconocido");
    }
}

void ServidorJuego::cmdHola(const shared_ptr<Jugador>& j, const string& nombre, bool& seguir) {
    if (fase != Fase::ESPERA) {
        enviar(j, "ERROR La partida ya comenzo; no se aceptan nuevos jugadores.");
        log.registrar(nombre, "-", "RECHAZADO", "ip=" + j->ip + ",motivo=partida_en_curso");
        seguir = false;
        return;
    }
    if (!nombreJugadorValido(nombre)) {
        enviar(j, "ERROR_NOMBRE Nombre invalido. Use de 1 a 20 caracteres: letras, numeros, '_', '-' o '.'. Ingrese otro nombre:");
        return;
    }
    for (const auto& otro : jugadores) {
        if (otro->conectado && otro->registrado && aMayusculas(otro->nombre) == aMayusculas(nombre)) {
            enviar(j, "ERROR_NOMBRE El nombre '" + nombre + "' ya esta en uso en esta partida. Ingrese otro nombre:");
            log.registrar(nombre, "-", "RECHAZADO", "ip=" + j->ip + ",motivo=nombre_en_uso");
            return;
        }
    }

    j->nombre = nombre;
    j->registrado = true;
    log.registrar(j->nombre, "-", "CONECTA", "ip=" + j->ip);

    enviar(j, "BIENVENIDO " + j->nombre);
    enviar(j, "INFO === CARRERA DE EQUIPOS === Tablero de " + to_string(cfg.tamanoTablero) +
              " posiciones, dado de " + to_string(cfg.carasDado) + " caras.");
    enviar(j, "INFO Gana el primer equipo que alcance o supere la posicion " + to_string(cfg.tamanoTablero) + ".");
    string letras;
    for (size_t i = 0; i < equipos.size(); ++i) { if (i) letras += ", "; letras += static_cast<char>('A' + i); }
    enviar(j, "INFO Elija equipo con: EQUIPO <letra>  (" + letras + ") y luego escriba LISTO.");
    difundir("INFO " + j->nombre + " se conecto al servidor.");
    difundirEspera();
}

void ServidorJuego::cmdEquipo(const shared_ptr<Jugador>& j, const string& arg) {
    if (fase != Fase::ESPERA) {
        enviar(j, "ERROR No se puede cambiar de equipo con la partida en curso.");
        log.registrar(j->nombre, nombreEquipoDe(j), "ACCION_RECHAZADA", "comando=EQUIPO,motivo=partida_en_curso");
        return;
    }
    // Acepta "A", "a", "EquipoA", "Equipo A" o el numero "1".
    string a = aMayusculas(arg);
    a.erase(remove(a.begin(), a.end(), ' '), a.end());
    if (a.rfind("EQUIPO", 0) == 0) a = a.substr(6);
    int idx = -1;
    if (a.size() == 1 && a[0] >= 'A' && a[0] < 'A' + static_cast<int>(equipos.size())) idx = a[0] - 'A';
    else if (!a.empty() && all_of(a.begin(), a.end(), [](char c) { return isdigit(static_cast<unsigned char>(c)) != 0; })
             && a.size() < 4) {
        int n = stoi(a);
        if (n >= 1 && n <= static_cast<int>(equipos.size())) idx = n - 1;
    }
    if (idx < 0) {
        enviar(j, "ERROR Equipo invalido. Use: EQUIPO <letra> (A.." + string(1, static_cast<char>('A' + equipos.size() - 1)) + ").");
        return;
    }
    if (j->equipo == idx) {
        enviar(j, "INFO Ya pertenece a " + equipos[idx].nombre + ".");
        return;
    }
    Equipo& destino = equipos[idx];
    if (static_cast<int>(destino.miembros.size()) >= cfg.maxJugadoresEquipo) {
        enviar(j, "ERROR " + destino.nombre + " esta completo (maximo " + to_string(cfg.maxJugadoresEquipo) + " jugadores por equipo).");
        log.registrar(j->nombre, destino.nombre, "ACCION_RECHAZADA", "comando=EQUIPO,motivo=equipo_completo");
        return;
    }
    string anterior = "-";
    if (j->equipo >= 0) {
        Equipo& viejo = equipos[j->equipo];
        anterior = viejo.nombre;
        viejo.miembros.erase(remove(viejo.miembros.begin(), viejo.miembros.end(), j->id), viejo.miembros.end());
    }
    destino.miembros.push_back(j->id);
    j->equipo = idx;
    j->listo = false;     // al cambiar de equipo debe confirmar nuevamente
    log.registrar(j->nombre, destino.nombre, "UNE_EQUIPO",
                  "integrantes=" + to_string(destino.miembros.size()) + (anterior != "-" ? ",equipo_anterior=" + anterior : ""));
    difundir("INFO " + j->nombre + " se unio a " + destino.nombre + " (" + to_string(destino.miembros.size()) + "/" +
             to_string(cfg.maxJugadoresEquipo) + ").");
    difundirEspera();
}

void ServidorJuego::cmdListo(const shared_ptr<Jugador>& j) {
    if (fase != Fase::ESPERA) {
        enviar(j, "ERROR La partida ya comenzo.");
        return;
    }
    if (j->equipo < 0) {
        enviar(j, "ERROR Primero elija un equipo con: EQUIPO <letra>.");
        log.registrar(j->nombre, "-", "ACCION_RECHAZADA", "comando=LISTO,motivo=sin_equipo");
        return;
    }
    if (j->listo) {
        enviar(j, "INFO Ya estaba marcado como listo.");
        return;
    }
    j->listo = true;
    log.registrar(j->nombre, nombreEquipoDe(j), "LISTO", "-");
    difundir("INFO " + j->nombre + " (" + nombreEquipoDe(j) + ") esta listo.");
    verificarInicio();
}

void ServidorJuego::cmdLanzar(const shared_ptr<Jugador>& j) {
    if (fase == Fase::ESPERA) {
        enviar(j, "ERROR La partida aun no comienza.");
        log.registrar(j->nombre, nombreEquipoDe(j), "ACCION_RECHAZADA", "comando=LANZAR,motivo=partida_no_iniciada");
        return;
    }
    if (fase == Fase::TERMINADA) {
        enviar(j, "ERROR La partida ya termino.");
        return;
    }

// -------------------------------------------------------------------------------------------------------------------------------------------------

    // 2.19) VALIDAR EL LANZAMIENTO
    // CONTROL DE TURNOS: una acción fuera de turno NO cambia el estado. Se rechaza, se avisa al jugador y queda registrada en el log.
    if (idTurno != j->id) {
        auto actual = buscarJugador(idTurno);
        enviar(j, "ERROR No es su turno. Turno de: " + (actual ? actual->nombre : string("?")) + ".");
        log.registrar(j->nombre, nombreEquipoDe(j), "ACCION_RECHAZADA", "comando=LANZAR,motivo=no_es_su_turno");
        return;
    }
    ejecutarLanzamiento(j, false);
}

void ServidorJuego::cmdChat(const shared_ptr<Jugador>& j, const string& texto) {
    if (texto.empty()) {
        enviar(j, "ERROR Uso: CHAT <mensaje>");
        return;
    }
    string t = texto.substr(0, 300);
    log.registrar(j->nombre, nombreEquipoDe(j), "CHAT", "mensaje=" + t);
    difundir("CHAT " + j->nombre + " [" + nombreEquipoDe(j) + "]: " + t);
}

void ServidorJuego::enviarEstado(const shared_ptr<Jugador>& j) {
    string f = fase == Fase::ESPERA ? "EN ESPERA" : (fase == Fase::EN_CURSO ? "EN CURSO" : "TERMINADA");
    enviar(j, "ESTADO Partida " + f + " | jugadores: " + to_string(jugadoresRegistrados()) + " | meta: posicion " +
              to_string(cfg.tamanoTablero) + " | dado: 1.." + to_string(cfg.carasDado));
    for (const auto& e : equipos) {
        if (fase != Fase::ESPERA && !e.participa) continue;
        string linea = "ESTADO   " + e.nombre + ": ";
        if (fase != Fase::ESPERA) linea += "posicion " + to_string(e.posicion) + "/" + to_string(cfg.tamanoTablero) +
                                           (e.eliminado ? " (ELIMINADO)" : "") + " | ";
        linea += describirEquipo(e);
        enviar(j, linea);
    }
    if (fase == Fase::EN_CURSO) {
        auto actual = buscarJugador(idTurno);
        enviar(j, "ESTADO Turno " + to_string(numeroTurno) + ": " + (actual ? actual->nombre : string("?")));
    }
}

void ServidorJuego::enviarAyuda(const shared_ptr<Jugador>& j) {
    enviar(j, "AYUDA EQUIPO <letra>  : unirse a un equipo (antes de iniciar)");
    enviar(j, "AYUDA LISTO           : confirmar que esta listo para comenzar");
    enviar(j, "AYUDA LANZAR          : lanzar el dado (solo en su turno)");
    enviar(j, "AYUDA CHAT <mensaje>  : enviar un mensaje a todos los jugadores");
    enviar(j, "AYUDA ESTADO          : ver el estado de la partida");
    enviar(j, "AYUDA SALIR           : abandonar la partida");
}

void ServidorJuego::verificarInicio() {
    if (fase != Fase::ESPERA) return;
    int registrados = 0, sinEquipo = 0, noListos = 0;
    for (const auto& j : jugadores) {
        if (!j->conectado || !j->registrado) continue;
        registrados++;
        if (j->equipo < 0) sinEquipo++;
        if (!j->listo) noListos++;
    }
    int conJugadores = 0, incompletos = 0;
    for (const auto& e : equipos) {
        if (e.miembros.empty()) continue;
        conJugadores++;
        if (static_cast<int>(e.miembros.size()) < cfg.minJugadoresEquipo) incompletos++;
    }

// -------------------------------------------------------------------------------------------------------------------------------------------------

    //1.17) LA CONDICION DE INICIO
    // CONDICIÓN DE INICIO (5 reglas explícitas):
    //  1. hay al menos GAME_MIN_PLAYERS jugadores
    //  2. todos tienen equipo
    //  3. hay al menos GAME_MIN_TEAMS equipos con jugadores
    //  4. cada equipo tiene su mínimo de jugadores
    //  5. TODOS escribieron LISTO
    // Se revisa cada vez que cambia la sala (entra alguien, cambia de equipo, se va).
    bool ok = registrados >= cfg.minJugadores && sinEquipo == 0 && conJugadores >= cfg.minEquipos &&
              incompletos == 0 && noListos == 0;
    if (ok) iniciarPartida();
    // Mientras no se cumpla, el servidor informa a todos QUÉ falta (mensaje ESPERA).
    else difundirEspera();
}

void ServidorJuego::difundirEspera() {
    if (fase != Fase::ESPERA) return;
    int registrados = 0, sinEquipo = 0, noListos = 0;
    for (const auto& j : jugadores) {
        if (!j->conectado || !j->registrado) continue;
        registrados++;
        if (j->equipo < 0) sinEquipo++;
        if (!j->listo) noListos++;
    }
    int conJugadores = 0;
    vector<string> incompletos;
    for (const auto& e : equipos) {
        if (e.miembros.empty()) continue;
        conJugadores++;
        if (static_cast<int>(e.miembros.size()) < cfg.minJugadoresEquipo) incompletos.push_back(e.nombre);
    }

    difundir("ESPERA ---- Sala de espera: " + to_string(registrados) + " jugador(es) [min " + to_string(cfg.minJugadores) +
             ", max " + to_string(cfg.maxJugadores) + "] ----");
    string vacios;
    for (const auto& e : equipos) {
        if (e.miembros.empty()) { vacios += (vacios.empty() ? "" : ", ") + e.nombre; continue; }
        difundir("ESPERA   " + e.nombre + " (" + to_string(e.miembros.size()) + "/" + to_string(cfg.maxJugadoresEquipo) +
                 "): " + describirEquipo(e));
    }
    if (!vacios.empty()) difundir("ESPERA   Equipos sin jugadores: " + vacios);
    vector<string> falta;
    if (registrados < cfg.minJugadores)
        falta.push_back("faltan " + to_string(cfg.minJugadores - registrados) + " jugador(es)");
    if (conJugadores < cfg.minEquipos)
        falta.push_back("se necesitan al menos " + to_string(cfg.minEquipos) + " equipos con jugadores (hay " +
                        to_string(conJugadores) + ")");
    for (const auto& n : incompletos)
        falta.push_back(n + " necesita al menos " + to_string(cfg.minJugadoresEquipo) + " jugador(es)");
    if (sinEquipo > 0) falta.push_back(to_string(sinEquipo) + " jugador(es) sin equipo");
    if (noListos > 0) falta.push_back(to_string(noListos) + " jugador(es) aun no estan LISTO");
    string msg;
    for (size_t i = 0; i < falta.size(); ++i) { if (i) msg += "; "; msg += falta[i]; }
    if (!msg.empty()) difundir("ESPERA Esperando para iniciar: " + msg + ".");
}

// ----------------------------------------------------------------------------
// Desarrollo de la partida
// ----------------------------------------------------------------------------
void ServidorJuego::iniciarPartida() {
    fase = Fase::EN_CURSO;
    int nEquipos = 0;
    string orden;
    for (auto& e : equipos) {
        e.participa = !e.miembros.empty();
        e.eliminado = false;
        e.posicion = 0;
        e.siguienteMiembro = 0;
        if (e.participa) {
            nEquipos++;
            if (!orden.empty()) orden += " -> ";
            orden += e.nombre;
        }
    }
    log.registrar("-", "-", "INICIA_PARTIDA",
                  "jugadores=" + to_string(jugadoresRegistrados()) + ",equipos=" + to_string(nEquipos) +
                  ",tablero=" + to_string(cfg.tamanoTablero) + ",dado=" + to_string(cfg.carasDado));
    difundir("INICIO ============================================");
    difundir("INICIO  LA PARTIDA COMIENZA! " + to_string(jugadoresRegistrados()) + " jugadores, " + to_string(nEquipos) + " equipos.");
    difundir("INICIO  Meta: posicion " + to_string(cfg.tamanoTablero) + ". Orden de turnos: " + orden + " (los integrantes se alternan).");
    if (cfg.segundosTurno > 0)
        difundir("INICIO  Cada jugador tiene " + to_string(cfg.segundosTurno) + " s para lanzar; si no, el servidor lanza por el.");
    difundir("INICIO ============================================");
    equipoTurno = -1;
    siguienteTurno();
}

void ServidorJuego::siguienteTurno() {
    if (fase != Fase::EN_CURSO) return;
    int n = static_cast<int>(equipos.size());
    for (int paso = 1; paso <= n; ++paso) {

// -------------------------------------------------------------------------------------------------------------------------------------------------

        // 2.18) ASIGNAR TURNOS
        // TURNOS: el servidor recorre los equipos en círculo (A -> B -> C -> ...). Dentro de cada equipo, alterna a sus 
        //integrantes con siguienteMiembro.
        int idx = (equipoTurno + paso + n) % n;     // siguiente equipo, en círculo
        Equipo& e = equipos[idx];
        if (!e.participa || e.eliminado || e.miembros.empty()) continue;
        if (e.siguienteMiembro >= e.miembros.size()) e.siguienteMiembro = 0;
        int idJugador = e.miembros[e.siguienteMiembro];     // quién le toca dentro del equipo
        e.siguienteMiembro = (e.siguienteMiembro + 1) % e.miembros.size();
        auto j = buscarJugador(idJugador);
        if (!j) continue;

        equipoTurno = idx;
        idTurno = idJugador;
        numeroTurno++;
        limiteTurno = Reloj::now() + chrono::seconds(cfg.segundosTurno);
        log.registrar(j->nombre, e.nombre, "TURNO", "numero=" + to_string(numeroTurno) + ",posicion_equipo=" + to_string(e.posicion));
        difundir("TURNO Turno " + to_string(numeroTurno) + " -> Turno de jugador: " + j->nombre + " (" + e.nombre +
                 ", posicion " + to_string(e.posicion) + ")");
        // Solo el jugador con el turno recibe TU_TURNO. Los demás reciben TURNO.
        enviar(j, "TU_TURNO Es su turno! Escriba LANZAR (o presione ENTER) para lanzar el dado de " +
                  to_string(cfg.carasDado) + " caras." +
                  (cfg.segundosTurno > 0 ? " Tiene " + to_string(cfg.segundosTurno) + " s." : string("")));
        return;
    }
    // No deberia ocurrir: no quedan equipos con jugadores.
    terminarPartida(-1, "-", "sin_jugadores");
}

void ServidorJuego::ejecutarLanzamiento(const shared_ptr<Jugador>& j, bool automatico) {
    Equipo& e = equipos[j->equipo];

// -------------------------------------------------------------------------------------------------------------------------------------------------

    //1.20) EL DADO, EL AVANCE Y EL CHOQUE
    // LANZAMIENTO: el SERVIDOR tira el dado (valor aleatorio entre 1 y R). El cliente solo pide lanzar; nunca decide el resultado.
    uniform_int_distribution<int> dado(1, cfg.carasDado);
    int valor = dado(rng);      // 1..R
    int desde = e.posicion;
    int hasta = min(desde + valor, cfg.tamanoTablero);      // no pasa de la meta

    log.registrar(j->nombre, e.nombre, "LANZA_DADO", "valor=" + to_string(valor) + (automatico ? ",automatico=si" : ""));
    difundir("RESULTADO " + j->nombre + " obtuvo: " + to_string(valor) + (automatico ? " (lanzamiento automatico por tiempo)" : ""));

    e.posicion = hasta;
    log.registrar(j->nombre, e.nombre, "AVANZA", "desde=" + to_string(desde) + ",hasta=" + to_string(hasta));
    difundir("EVENTO " + e.nombre + " avanzo desde la posicion " + to_string(desde) + " hasta la posicion " + to_string(hasta) + ".");

//-------------------------------------------------------------------------------------------------------------------------------------------

    //2.21) FIN DE LA PARTIDA
    // CONDICIÓN DE TÉRMINO: el primer equipo que alcanza o supera la meta gana.
    if (hasta >= cfg.tamanoTablero) {
        terminarPartida(j->equipo, j->nombre, "meta_alcanzada");
        return;
    }
    // Interaccion entre equipos: si se cae exactamente en la casilla de un
    // rival, el rival es empujado hacia atras tantas posiciones como el dado.
    for (auto& rival : equipos) {
        if (&rival == &e || !rival.participa || rival.eliminado) continue;
        
        // REGLA DE CHOQUE: si un equipo cae EXACTAMENTE en la casilla de un rival, el rival retrocede tantas posiciones 
        //como el dado. (Regla propia del grupo.)
        if (rival.posicion == hasta && hasta > 0) {     // cayó en casilla rival
            int antes = rival.posicion;
            rival.posicion = max(0, rival.posicion - valor);
            log.registrar(j->nombre, e.nombre, "CHOQUE",
                          "rival=" + rival.nombre + ",rival_desde=" + to_string(antes) + ",rival_hasta=" + to_string(rival.posicion));
            difundir("EVENTO CHOQUE! " + e.nombre + " cayo en la casilla de " + rival.nombre + ": " + rival.nombre +
                     " retrocede de la posicion " + to_string(antes) + " a la " + to_string(rival.posicion) + ".");
        }
    }

    string tablero;
    for (const auto& eq : equipos) {
        if (!eq.participa || eq.eliminado) continue;
        if (!tablero.empty()) tablero += " | ";
        tablero += eq.nombre + "=" + to_string(eq.posicion);
    }
    difundir("TABLERO " + tablero + "  (meta " + to_string(cfg.tamanoTablero) + ")");
    siguienteTurno();
}

void ServidorJuego::terminarPartida(int equipoGanador, const string& jugadorGanador, const string& motivo) {
    if (fase == Fase::TERMINADA) return;
    fase = Fase::TERMINADA;
    idTurno = -1;

    if (equipoGanador >= 0) {
        const Equipo& g = equipos[equipoGanador];
        log.registrar(jugadorGanador, g.nombre, "GANA", "motivo=" + motivo + ",posicion=" + to_string(g.posicion));
    }
    log.registrar("-", "-", "FIN_PARTIDA", "motivo=" + motivo + ",turnos=" + to_string(numeroTurno));


    vector<int> orden;
    for (int i = 0; i < static_cast<int>(equipos.size()); ++i) if (equipos[i].participa) orden.push_back(i);
    stable_sort(orden.begin(), orden.end(), [&](int a, int b) {
        if (a == equipoGanador) return b != equipoGanador;
        if (b == equipoGanador) return false;
        return equipos[a].posicion > equipos[b].posicion;
    });

    difundir("FIN ============================================");
    if (equipoGanador >= 0) {
        string porque = motivo == "meta_alcanzada"
                            ? " (" + jugadorGanador + " alcanzo la posicion " + to_string(cfg.tamanoTablero) + ")"
                            : " (los demas equipos abandonaron)";
        difundir("FIN  PARTIDA TERMINADA. GANADOR: " + equipos[equipoGanador].nombre + porque);
    } else {
        difundir("FIN  PARTIDA TERMINADA sin ganador (" + motivo + ").");
    }
    int lugar = 1;
    for (int idx : orden) {
        const Equipo& e = equipos[idx];
        log.registrar("-", e.nombre, "RESULTADO",
                      "lugar=" + to_string(lugar) + ",posicion=" + to_string(e.posicion) + (e.eliminado ? ",eliminado=si" : ""));
        difundir("FIN  " + to_string(lugar) + "o lugar: " + e.nombre + " - posicion " + to_string(e.posicion) +
                 (e.eliminado ? " (sin jugadores)" : "") + " - " + describirEquipo(e));
        lugar++;
    }
    difundir("FIN  El servidor cerrara la conexion. Gracias por jugar!");
    difundir("FIN ============================================");
    // Se avisa al hilo principal que debe cerrar el servidor de forma controlada.
    detener = true;     // el hilo principal hace el cierre controlado
}

//-------------------------------------------------------------------------------------------------------------------------------------------

// 2.29) MANEJO DE DESCONEXIONES
// Una desconexion NO tumba el servidor. Se quita al jugador de la partida,
// se registra en el log y el juego sigue con los demas.
void ServidorJuego::manejarDesconexion(const shared_ptr<Jugador>& j, const string& motivo) {
    // La caida se detecta en atenderCliente: leerLinea() devuelve 0 (cierre) o un valor negativo (error de conexion). Aqui se 
    // marca al jugador como desconectado una sola vez, aunque el hecho se llame varias veces.
    if (!j->conectado) return;
    j->conectado = false;

    if (!j->registrado) {
        log.registrar("-", "-", "DESCONECTA", "ip=" + j->ip + ",motivo=" + motivo + ",identificado=no");
        return;
    }
    string equipo = nombreEquipoDe(j);
    // Punto 2.12 y 2.13: el evento DESCONECTA queda en el log con jugador, equipo y motivo.
    log.registrar(j->nombre, equipo, "DESCONECTA", "motivo=" + motivo);
    if (fase == Fase::TERMINADA) return;

    difundir("INFO " + j->nombre + " se desconecto (" + motivo + ").");

    // Se quita al jugador de la lista de miembros de su equipo. Si el equipo se queda sin jugadores durante la partida, queda ELIMINADO.
    // Un equipo eliminado no recibe turnos y no compite por la meta.
    if (j->equipo >= 0) {
        Equipo& e = equipos[j->equipo];
        auto it = find(e.miembros.begin(), e.miembros.end(), j->id);
        if (it != e.miembros.end()) {
            size_t pos = static_cast<size_t>(it - e.miembros.begin());
            e.miembros.erase(it);
            if (pos < e.siguienteMiembro) e.siguienteMiembro--;
            if (e.siguienteMiembro >= e.miembros.size()) e.siguienteMiembro = 0;
        }
        if (fase == Fase::EN_CURSO && e.participa && e.miembros.empty()) {
            e.eliminado = true;
            log.registrar("-", e.nombre, "EQUIPO_ELIMINADO", "motivo=sin_jugadores,posicion=" + to_string(e.posicion));
            difundir("EVENTO " + e.nombre + " se quedo sin jugadores y queda fuera de la partida.");
        }
    }

    // Si la partida aun no empieza, solo se actualiza la sala y se reevalua la condicion de inicio (verificarInicio). 
    // Puede que ahora se cumpla o que falte alguien.
    if (fase == Fase::ESPERA) {
        j->equipo = -1;
        verificarInicio();   // puede que ahora se cumpla la condicion (ej: se fue el unico no listo)
        return;
    }

    // Partida en curso: si solo queda un equipo activo, esa partida termina por abandono de los rivales. Asi el servidor 
    // nunca queda esperando a un equipo vacio.
    int activos = equiposActivos();
    if (activos <= 1) {
        int ganador = -1;
        for (int i = 0; i < static_cast<int>(equipos.size()); ++i)
            if (equipos[i].participa && !equipos[i].eliminado) ganador = i;
        terminarPartida(ganador, "-", ganador >= 0 ? "abandono_rivales" : "sin_jugadores");
        return;
    }

    // Si el jugador que se fue tenia el turno, el turno pasa al siguiente. Nadie queda bloqueado esperando una accion que nunca llegara.
    if (idTurno == j->id) {
        difundir("INFO El turno de " + j->nombre + " pasa al siguiente jugador.");
        siguienteTurno();
    }
}

// -------------------------------------------------------------------------------------------------------------------------------------------------

// 2.15) EL THREAD DE CADA CLIENTE
void ServidorJuego::atenderCliente(shared_ptr<Jugador> j) {
    LectorLineas lector(j->sock);
    string linea;
    string motivo = "cierre_cliente";
    bool seguir = true;
    while (seguir) {
        int r = lector.leerLinea(linea);
        // Si el cliente se desconecta, el thread termina limpio. NO se cae el servidor: los demás jugadores siguen jugando
        if (r == 0) { motivo = "cierre_cliente"; break; }
        if (r < 0) { motivo = "conexion_perdida"; break; }

        // SECCIÓN CRÍTICA 3: cada comando se aplica con el estado bloqueado. Así dos jugadores nunca modifican la partida 
        // al mismo tiempo (por ejemplo, dos lanzamientos simultáneos)
        lock_guard<mutex> guardia(mtxEstado);       // ── SECCIÓN CRÍTICA 3
        if (detener) { motivo = "cierre_servidor"; break; }
        procesarComando(j, linea, seguir);
        if (!seguir) motivo = "salida_voluntaria";
    }
    {
        lock_guard<mutex> guardia(mtxEstado);
        if (detener && motivo != "salida_voluntaria") motivo = "cierre_servidor";
        manejarDesconexion(j, motivo);
    }
    redApagar(j->sock);   // el descriptor se libera en el cierre controlado
}

// -------------------------------------------------------------------------------------------------------------------------------------------------

// Controla el tiempo maximo de cada turno (GAME_TURN_TIMEOUT).
void ServidorJuego::temporizador() {
    
    //2.22) EL TEMPORIZADOR
    // TEMPORIZADOR: otro thread. Cada 200 ms revisa si venció el turno. Si venció, el servidor lanza el dado por el jugador.
    while (!detener && !senalTerminar) {
        this_thread::sleep_for(chrono::milliseconds(200));
        if (cfg.segundosTurno <= 0) continue;
        
        // También modifica el estado (posiciones, turno), así que toma el mismo candado.
        lock_guard<mutex> guardia(mtxEstado);

        if (fase != Fase::EN_CURSO || Reloj::now() < limiteTurno) continue;
        auto j = buscarJugador(idTurno);
        if (!j || !j->conectado) { siguienteTurno(); continue; }
        log.registrar(j->nombre, nombreEquipoDe(j), "TIEMPO_AGOTADO", "segundos=" + to_string(cfg.segundosTurno));
        difundir("INFO " + j->nombre + " no lanzo a tiempo: el servidor lanza el dado por el.");
        ejecutarLanzamiento(j, true);
    }
}

// ----------------------------------------------------------------------------
// Bucle principal y cierre controlado
// ----------------------------------------------------------------------------
int ServidorJuego::ejecutar() {
    string error;
    if (!log.abrir(cfg.archivoLog, error)) {
        cerr << "Error: " << error << "\n";
        return 1;
    }
    if (!redInicializar()) {
        cerr << "Error: no se pudo inicializar la biblioteca de sockets.\n";
        log.cerrar();
        return 1;
    }
    socket_t escucha = redCrearServidor(cfg.conexion.puerto, error);
    if (escucha == SOCKET_INVALIDO) {
        cerr << "Error: " << error << "\n";
        log.registrar("-", "-", "ERROR_SERVIDOR", error);
        log.cerrar();
        redFinalizar();
        return 1;
    }

    signal(SIGINT, manejadorSenal);
    signal(SIGTERM, manejadorSenal);

    cout << "============================================================\n";
    cout << "  SERVIDOR - CARRERA DE EQUIPOS (escuchando en el puerto " << cfg.conexion.puerto << ")\n";
    cout << "  Tablero: " << cfg.tamanoTablero << " posiciones | Dado: " << cfg.carasDado << " caras\n";
    cout << "  Jugadores: " << cfg.minJugadores << " a " << cfg.maxJugadores << " | Equipos: " << cfg.minEquipos
         << " a " << cfg.maxEquipos << " | Jugadores por equipo: " << cfg.minJugadoresEquipo << " a "
         << cfg.maxJugadoresEquipo << "\n";
    cout << "  Log: " << cfg.archivoLog << "   (Ctrl+C para detener el servidor)\n";
    cout << "============================================================\n";
    log.registrar("-", "-", "SERVIDOR_INICIA",
                  "puerto=" + to_string(cfg.conexion.puerto) + ",tablero=" + to_string(cfg.tamanoTablero) +
                  ",dado=" + to_string(cfg.carasDado) + ",jugadores=" + to_string(cfg.minJugadores) + "-" +
                  to_string(cfg.maxJugadores) + ",equipos=" + to_string(cfg.minEquipos) + "-" + to_string(cfg.maxEquipos) +
                  ",por_equipo=" + to_string(cfg.minJugadoresEquipo) + "-" + to_string(cfg.maxJugadoresEquipo));

// -------------------------------------------------------------------------------------------------------------------------------------------------

    thread hiloTemporizador(&ServidorJuego::temporizador, this);
    int conexionesAtendidas = 0;
    // 2.13) EL SERVIDOR ACEPTA JUGADORES  
    // PROBLEMA 2 / SERVIDOR: el hilo principal solo ACEPTA conexiones. Cada cliente recibe su PROPIO thread (atenderCliente)
    while (!detener && !senalTerminar) {
        int r = redEsperarActividad(escucha, 300);
        if (r <= 0) continue;
        string ip;
        socket_t cliente = redAceptar(escucha, ip);
        if (cliente == SOCKET_INVALIDO) continue;

        lock_guard<mutex> guardia(mtxEstado);
        if (fase != Fase::ESPERA || detener) {
            redEnviarLinea(cliente, "ERROR La partida ya comenzo; no se aceptan nuevos jugadores.");
            log.registrar("-", "-", "RECHAZADO", "ip=" + ip + ",motivo=partida_en_curso");
            redCerrar(cliente);
            continue;
        }
        if (conexionesAbiertas() >= cfg.maxJugadores) {
            redEnviarLinea(cliente, "ERROR La sala esta llena (maximo " + to_string(cfg.maxJugadores) + " jugadores).");
            log.registrar("-", "-", "RECHAZADO", "ip=" + ip + ",motivo=sala_llena");
            redCerrar(cliente);
            continue;
        }
        
        auto j = make_shared<Jugador>();
        j->id = siguienteId++;
        j->sock = cliente;
        j->ip = ip;
        jugadores.push_back(j);
        conexionesAtendidas++;
        redEnviarLinea(cliente, "INFO Conectado al servidor de Carrera de Equipos. Identifiquese con: HOLA <nombre>");
        // Un thread por jugador: varios clientes se atienden al mismo tiempo
        hilosClientes.emplace_back(&ServidorJuego::atenderCliente, this, j);
    }

    // ---------------- Cierre controlado de recursos ----------------
    {
        lock_guard<mutex> guardia(mtxEstado);
        if (senalTerminar && fase != Fase::TERMINADA) {
            log.registrar("-", "-", "SERVIDOR_INTERRUMPIDO", "motivo=senal_de_termino");
            difundir("FIN El servidor fue detenido por el administrador. La partida se cancela.");
            fase = Fase::TERMINADA;
        }
        detener = true;
    }
    this_thread::sleep_for(chrono::milliseconds(300));   // deja salir los ultimos mensajes
    
//------------------------------------------------------------------------------------------------------------------------    

    //2.24) CIERRE CONTROLADO
    {
        lock_guard<mutex> guardia(mtxEstado);

        // CIERRE CONTROLADO en 4 pasos, en este orden:
        // 1) apagar sockets: así los recv() bloqueados de cada thread terminan
        // 2) esperar (join) a todos los threads
        // 3) cerrar descriptores y vaciar las estructuras compartidas
        // 4) registrar el cierre en el log
        for (const auto& j : jugadores) if (j->conectado) redApagar(j->sock); // 1) sockets
    }   
    // 2) threads: se espera a que todos terminen
    for (auto& h : hilosClientes) if (h.joinable()) h.join();                   // 2) threads
    if (hiloTemporizador.joinable()) hiloTemporizador.join();                  

    // 3) se liberan los descriptores de socket y las estructuras compartidas
    size_t total = jugadores.size();
    for (const auto& j : jugadores) { redCerrar(j->sock); j->sock = SOCKET_INVALIDO; }// 3) descriptores
    jugadores.clear();                                                              //    estructuras
    for (auto& e : equipos) e.miembros.clear();
    redCerrar(escucha);
    redFinalizar();

    // 4) archivo de log
    log.registrar("-", "-", "SERVIDOR_CIERRA",                                       // 4) log
                  "conexiones_atendidas=" + to_string(conexionesAtendidas) + ",threads_finalizados=" + to_string(total + 1));
    log.cerrar();
    cout << "Servidor finalizado correctamente. Recursos liberados.\n";
    return 0;
}

int main(int argc, char* argv[]) {
    // Permite lanzar el servidor desde otra carpeta (busca .env junto al ejecutable).
    string carpetaPrograma;
    if (ubicarCarpetaDelPrograma(argc > 0 ? argv[0] : nullptr, carpetaPrograma))
        cout << "Nota: se usa la carpeta del programa: " << carpetaPrograma << "\n";
    cargarVariablesEntorno(".env");

    ConfigJuego cfg;
    string error;
    if (!cargarConfigJuego(cfg, error)) {
        cerr << "Error de configuracion: " << error << "\n";
        cerr << "Revise el archivo .env o las variables de entorno (ver README).\n";
        return 1;
    }

    ServidorJuego servidor(cfg);
    return servidor.ejecutar();
}
