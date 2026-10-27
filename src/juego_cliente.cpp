// ============================================================================
// ENTREGA 3 - PROBLEMA 2: CLIENTE del juego multiplayer "Carrera de Equipos"
// ----------------------------------------------------------------------------
// El cliente usa DOS threads:
//   - thread receptor : lee continuamente los mensajes del servidor (socket)
//                       y los muestra en pantalla en tiempo real.
//   - thread principal: lee lo que escribe el jugador por teclado y lo envia
//                       al servidor como un comando del protocolo.
// Asi el jugador ve lo que hacen los demas aunque no este escribiendo.
// ============================================================================

#include "../include/juego.h"
#include "../include/red.h"
#include "../include/env.h"

#include <atomic>
#include <cctype>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

using namespace std;

namespace {

mutex mtxPantalla;   
// mtxPantalla: ambos threads escriben en la consola, asi que se protege
// para que los mensajes no se mezclen (misma idea que semConsola en el indice).
void imprimir(const string& texto) {
    lock_guard<mutex> guardia(mtxPantalla);
    cout << texto << endl;
}

string mayus(string s) {
    for (char& c : s) c = static_cast<char>(toupper(static_cast<unsigned char>(c)));
    return s;
}

string recortar(const string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

struct EstadoCliente {
    atomic<bool> registrado{false};   // el servidor acepto nuestro nombre
    atomic<bool> miTurno{false};      // el servidor nos dio el turno
    atomic<bool> terminado{false};    // la conexion finalizo
};

void mostrarAyudaLocal() {
    imprimir("  Comandos:  equipo <letra> | listo | lanzar (o ENTER en su turno) | chat <mensaje> | estado | ayuda | salir");
}

// Thread receptor: muestra los mensajes del servidor segun su tipo.
void receptor(socket_t sock, EstadoCliente* estado) {
    LectorLineas lector(sock);
    string linea;
    // El receptor vive bloqueado en leerLinea() esperando mensajes del servidor.
// Segun el tipo (BIENVENIDO, TURNO, TU_TURNO, FIN, CHAT...) los muestra.
// Cuando llega TU_TURNO marca miTurno = true: es la señal que usa el hilo
// principal para saber si ENTER significa "lanzar".
    while (true) {
        int r = lector.leerLinea(linea);
        if (r <= 0) break;
        size_t esp = linea.find(' ');
        string tipo = esp == string::npos ? linea : linea.substr(0, esp);
        string texto = esp == string::npos ? "" : linea.substr(esp + 1);

        if (tipo == "BIENVENIDO") {
            estado->registrado = true;
            imprimir("\n*** Bienvenido/a " + texto + "! Conectado a la partida. ***");
            mostrarAyudaLocal();
        } else if (tipo == "ERROR_NOMBRE") {
            imprimir("[ERROR] " + texto);
        } else if (tipo == "TURNO") {
            estado->miTurno = false;
            imprimir("\n[TURNO] " + texto);
        } else if (tipo == "TU_TURNO") {
            estado->miTurno = true;
            imprimir(">>>>>> " + texto + " <<<<<<");
        } else if (tipo == "FIN") {
            imprimir("[FIN] " + texto);
        } else if (tipo == "CHAT") {
            imprimir("  (chat) " + texto);
        } else {
            imprimir("[" + tipo + "] " + texto);
        }
    }
    estado->terminado = true;
    imprimir("\n--- Conexion con el servidor finalizada. Presione ENTER para continuar ---");
}

} // namespace

int ejecutarClienteJuego(const string& nombreJugador) {
    ConfigConexion cfg;
    string error;
    if (!cargarConfigConexion(cfg, error)) {
        cout << "\nError de configuracion: " << error << "\n";
        return 1;
    }
    if (!redInicializar()) {
        cout << "\nError: no se pudo inicializar la biblioteca de sockets.\n";
        return 1;
    }

    cout << "\nConectando con el servidor " << cfg.ipServidor << ":" << cfg.puerto << " ...\n";
    socket_t sock = redConectar(cfg.ipServidor, cfg.puerto, error);
    if (sock == SOCKET_INVALIDO) {
        cout << "Error: " << error << "\n";
        redFinalizar();
        return 1;
    }

//-----------------------------------------------------------------------------------------------------------------------

    //2.26) CLIENTE CON DOS THREADS:
    //  - receptor (este thread nuevo): muestra en tiempo real lo que envia el servidor
    //  - hilo principal (mas abajo): lee lo que escribe el jugador y lo envia
    // Asi el jugador ve lo que hacen los demas aunque no este escribiendo.
    EstadoCliente estado;
    thread hiloReceptor(receptor, sock, &estado);    // thread 2: recibe del servidor

    // Identificacion automatica con el nombre recibido.
    string nombre = nombreJugador;
    if (!nombreJugadorValido(nombre)) {
        imprimir("El nombre '" + nombre + "' no es valido para el juego. Ingrese otro nombre:");
    } else {
        redEnviarLinea(sock, "HOLA " + nombre);
    }

    // Bucle de entrada del jugador
    string entrada;
    while (!estado.terminado) {
        if (!getline(cin, entrada)) {                 // fin de la entrada estandar
            redEnviarLinea(sock, "SALIR");
            break;
        }
        if (estado.terminado) break;
        string e = recortar(entrada);

        if (!estado.registrado) {                    // aun no aceptan nuestro nombre
            if (e.empty()) continue;
            if (mayus(e) == "SALIR") { redEnviarLinea(sock, "SALIR"); break; }
            redEnviarLinea(sock, "HOLA " + e);
            continue;
        }

        string cmd = mayus(e.substr(0, e.find(' ')));
        string resto = e.find(' ') == string::npos ? "" : recortar(e.substr(e.find(' ') + 1));

        if (e.empty()) {
            
            // ENTER en su turno = lanzar el dado (atajo para el jugador).
            if (estado.miTurno) redEnviarLinea(sock, "LANZAR"); // ENTER = lanzar
            continue;
        }
        if (cmd == "L") cmd = "LANZAR";
        if (cmd == "E") cmd = "EQUIPO";
        if (cmd == "AYUDA" || cmd == "?") {
            mostrarAyudaLocal();
            continue;
        }
        string mensaje = resto.empty() ? cmd : cmd + " " + resto;
        if (!redEnviarLinea(sock, mensaje)) break;
        if (cmd == "SALIR") break;
    }

    // Cierre ordenado: se avisa al servidor, se espera al thread receptor y
    // se libera el socket.
    redApagar(sock);
    if (hiloReceptor.joinable()) hiloReceptor.join();
    redCerrar(sock);
    redFinalizar();
    cout << "Saliste del juego.\n";
    return 0;
}

