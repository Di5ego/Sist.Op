#ifndef RED_H
#define RED_H

// ============================================================================
// ENTREGA 3 - PROBLEMA 2: capa de comunicacion mediante sockets TCP
// ----------------------------------------------------------------------------
// Encapsula las diferencias entre los sockets de Berkeley (Linux/Unix/macOS)
// y Winsock2 (Windows) para que el servidor y el cliente del juego usen una
// unica interfaz. El protocolo del juego es de texto: cada mensaje es UNA
// linea terminada en '\n'.
// ============================================================================

#if defined(_WIN32) && !defined(_WIN32_WINNT)
  #define _WIN32_WINNT 0x0601   // Windows 7+: inet_ntop / getaddrinfo
#endif

#include <string>

#if defined(_WIN32)
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
  #include <winsock2.h>
  #include <ws2tcpip.h>
  typedef SOCKET socket_t;
  #define SOCKET_INVALIDO INVALID_SOCKET
#else
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <unistd.h>
  typedef int socket_t;
  #define SOCKET_INVALIDO (-1)
#endif

// Inicializa la biblioteca de sockets (WSAStartup en Windows; en Linux
// ademas ignora SIGPIPE para que escribir en un socket cerrado no mate al
// proceso). Devuelve false si no fue posible.
bool redInicializar();

// Libera la biblioteca de sockets (WSACleanup en Windows).
void redFinalizar();

// Cierra un socket (close / closesocket).
void redCerrar(socket_t s);

// Interrumpe la comunicacion en ambos sentidos (desbloquea un recv() que
// este esperando en otro thread) sin liberar aun el descriptor.
void redApagar(socket_t s);

// Envia una linea completa (agrega '\n'). Devuelve false si la conexion fallo.
bool redEnviarLinea(socket_t s, const std::string& linea);

// Crea un socket servidor escuchando en todas las interfaces en 'puerto'.
socket_t redCrearServidor(int puerto, std::string& error);

// Conecta con un servidor (ip o nombre de host) y puerto.
socket_t redConectar(const std::string& host, int puerto, std::string& error);

// Espera hasta 'milisegundos' a que haya una conexion entrante (o datos) en
// el socket. Devuelve 1 si hay actividad, 0 si vencio el tiempo, -1 si error.
int redEsperarActividad(socket_t s, int milisegundos);

// Acepta una conexion; devuelve la IP del cliente en 'ipCliente'.
socket_t redAceptar(socket_t servidor, std::string& ipCliente);

// Mensaje del ultimo error de sockets del sistema.
std::string redUltimoError();

//-------------------------------------------------------------------------------------------------------------------------

// 2.25) LA CAPA DE SOCKETS
// Lector de lineas sobre un socket TCP: un recv() puede traer media linea o
// varias lineas juntas, por lo que se acumula en un buffer y se entregan de a
// una. Lineas excesivamente largas se consideran un error de protocolo.
class LectorLineas { // acumula bytes hasta tener una línea completa
public:
    // 2.25)TCP entrega un FLUJO de bytes: no respeta los límites de los mensajes.
    // Un recv() puede traer media línea o tres líneas juntas.
    // Por eso el protocolo usa UNA línea por mensaje (terminada en \n) y
    // LectorLineas reconstruye cada mensaje completo antes de procesarlo.
    explicit LectorLineas(socket_t s) : sock(s) {}
    // 1 = linea leida, 0 = la otra parte cerro la conexion, -1 = error
    int leerLinea(std::string& linea);
private:
    socket_t sock;
    std::string buffer;
};

#endif // RED_H
