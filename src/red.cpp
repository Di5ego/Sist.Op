// ============================================================================
// ENTREGA 3 - PROBLEMA 2: implementacion de la capa de sockets (ver red.h)
// ============================================================================

#include "../include/red.h"

#include <cstring>
#include <cerrno>

#if !defined(_WIN32)
  #include <csignal>
  #include <sys/select.h>
  #include <sys/time.h>
#endif

using namespace std;

// Largo maximo aceptado para una linea del protocolo (protege al servidor de
// clientes que envien datos sin fin de linea).
static const size_t LARGO_MAXIMO_LINEA = 4096;

bool redInicializar() {
#if defined(_WIN32)
    WSADATA datos;
    return WSAStartup(MAKEWORD(2, 2), &datos) == 0;
#else
    signal(SIGPIPE, SIG_IGN);
    return true;
#endif
}

void redFinalizar() {
#if defined(_WIN32)
    WSACleanup();
#endif
}

void redCerrar(socket_t s) {
    if (s == SOCKET_INVALIDO) return;
#if defined(_WIN32)
    closesocket(s);
#else
    close(s);
#endif
}

void redApagar(socket_t s) {
    if (s == SOCKET_INVALIDO) return;
#if defined(_WIN32)
    shutdown(s, SD_BOTH);
#else
    shutdown(s, SHUT_RDWR);
#endif
}

string redUltimoError() {
#if defined(_WIN32)
    return "codigo Winsock " + to_string(WSAGetLastError());
#else
    return strerror(errno);
#endif
}

bool redEnviarLinea(socket_t s, const string& linea) {
    string datos = linea + "\n";
    size_t enviados = 0;
    while (enviados < datos.size()) {
#if defined(_WIN32)
        int n = send(s, datos.data() + enviados, static_cast<int>(datos.size() - enviados), 0);
#else
        ssize_t n = send(s, datos.data() + enviados, datos.size() - enviados, MSG_NOSIGNAL);
        if (n < 0 && errno == EINTR) continue;
#endif
        if (n <= 0) return false;
        enviados += static_cast<size_t>(n);
    }
    return true;
}

socket_t redCrearServidor(int puerto, string& error) {
    socket_t s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == SOCKET_INVALIDO) {
        error = "no se pudo crear el socket (" + redUltimoError() + ")";
        return SOCKET_INVALIDO;
    }
#if !defined(_WIN32)
    // Permite reutilizar el puerto inmediatamente despues de cerrar el servidor.
    int si = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &si, sizeof(si));
#endif
    sockaddr_in dir;
    memset(&dir, 0, sizeof(dir));
    dir.sin_family = AF_INET;
    dir.sin_addr.s_addr = htonl(INADDR_ANY);
    dir.sin_port = htons(static_cast<unsigned short>(puerto));

    if (::bind(s, reinterpret_cast<sockaddr*>(&dir), sizeof(dir)) != 0) {
        error = "no se pudo asociar el puerto " + to_string(puerto) + " (" + redUltimoError()
                + "). Puede que otro programa ya lo este usando.";
        redCerrar(s);
        return SOCKET_INVALIDO;
    }
    if (listen(s, 16) != 0) {
        error = "no se pudo escuchar en el puerto " + to_string(puerto) + " (" + redUltimoError() + ")";
        redCerrar(s);
        return SOCKET_INVALIDO;
    }
    return s;
}

socket_t redConectar(const string& host, int puerto, string& error) {
    addrinfo pistas;
    memset(&pistas, 0, sizeof(pistas));
    pistas.ai_family = AF_INET;
    pistas.ai_socktype = SOCK_STREAM;
    pistas.ai_protocol = IPPROTO_TCP;

    addrinfo* resultado = nullptr;
    string puertoStr = to_string(puerto);
    if (getaddrinfo(host.c_str(), puertoStr.c_str(), &pistas, &resultado) != 0 || resultado == nullptr) {
        error = "no se pudo resolver la direccion '" + host + "'";
        return SOCKET_INVALIDO;
    }

    socket_t s = SOCKET_INVALIDO;
    for (addrinfo* p = resultado; p != nullptr; p = p->ai_next) {
        s = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (s == SOCKET_INVALIDO) continue;
        if (connect(s, p->ai_addr, static_cast<int>(p->ai_addrlen)) == 0) break;
        redCerrar(s);
        s = SOCKET_INVALIDO;
    }
    freeaddrinfo(resultado);

    if (s == SOCKET_INVALIDO) {
        error = "no fue posible conectar con " + host + ":" + puertoStr +
                " (" + redUltimoError() + "). Verifique que el servidor del juego este iniciado.";
    }
    return s;
}

int redEsperarActividad(socket_t s, int milisegundos) {
    fd_set conjunto;
    FD_ZERO(&conjunto);
    FD_SET(s, &conjunto);
    timeval tv;
    tv.tv_sec = milisegundos / 1000;
    tv.tv_usec = (milisegundos % 1000) * 1000;
#if defined(_WIN32)
    int r = select(0, &conjunto, nullptr, nullptr, &tv);
#else
    int r = select(s + 1, &conjunto, nullptr, nullptr, &tv);
    if (r < 0 && errno == EINTR) return 0;   // interrumpido por una senal (ej: Ctrl+C)
#endif
    if (r < 0) return -1;
    return r > 0 ? 1 : 0;
}

socket_t redAceptar(socket_t servidor, string& ipCliente) {
    sockaddr_in dir;
    memset(&dir, 0, sizeof(dir));
#if defined(_WIN32)
    int largo = sizeof(dir);
#else
    socklen_t largo = sizeof(dir);
#endif
    socket_t c = accept(servidor, reinterpret_cast<sockaddr*>(&dir), &largo);
    if (c != SOCKET_INVALIDO) {
        // inet_ntoa usa un buffer estatico: solo lo llama el thread que acepta conexiones.
        ipCliente = inet_ntoa(dir.sin_addr);
    }
    return c;
}

int LectorLineas::leerLinea(string& linea) {
    while (true) {
        size_t pos = buffer.find('\n');
        if (pos != string::npos) {
            linea = buffer.substr(0, pos);
            buffer.erase(0, pos + 1);
            if (!linea.empty() && linea.back() == '\r') linea.pop_back();   // clientes Windows / telnet
            return 1;
        }
        if (buffer.size() > LARGO_MAXIMO_LINEA) return -1;

        char tmp[1024];
#if defined(_WIN32)
        int n = recv(sock, tmp, sizeof(tmp), 0);
#else
        ssize_t n = recv(sock, tmp, sizeof(tmp), 0);
        if (n < 0 && errno == EINTR) continue;
#endif
        if (n == 0) return 0;     // la otra parte cerro la conexion de forma ordenada
        if (n < 0) return -1;     // error / conexion interrumpida
        buffer.append(tmp, static_cast<size_t>(n));
    }
}
