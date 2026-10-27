#ifndef SEMAFORO_H
#define SEMAFORO_H

// ============================================================================
// ENTREGA 3 - Semaforo (wrapper portable sobre semaforos del sistema operativo)
// ----------------------------------------------------------------------------
// Encapsula un semaforo contador de Dijkstra con sus dos operaciones clasicas:
//   esperar()  -> P / wait / down : decrementa; si el valor es 0 el thread se
//                 BLOQUEA (sin consumir CPU) hasta que otro thread haga senalar().
//   senalar()  -> V / signal / post / up : incrementa y despierta a un thread
//                 bloqueado, si existe.
//
// Implementacion segun plataforma (siempre un semaforo nativo del SO):
//   - Linux / WSL / Unix : POSIX  sem_t  (sem_init / sem_wait / sem_post)
//   - Windows (MinGW)    : Win32  CreateSemaphore / WaitForSingleObject /
//                          ReleaseSemaphore
//   - macOS              : GCD dispatch_semaphore (sem_init no existe en macOS)
//
// Un Semaforo inicializado en 1 se usa como semaforo BINARIO (exclusion mutua
// de una seccion critica). Inicializado en 0 se usa como semaforo CONTADOR de
// recursos disponibles (ej: libros pendientes en la cola de trabajo).
// ============================================================================

#if defined(_WIN32) && !defined(_WIN32_WINNT)
  #define _WIN32_WINNT 0x0601   // Windows 7+: inet_ntop / getaddrinfo
#endif

#if defined(_WIN32)
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
  #include <winsock2.h>   // debe ir antes de windows.h para evitar conflictos con winsock.h
  #include <windows.h>
#elif defined(__APPLE__)
  #include <dispatch/dispatch.h>
#else
  #include <semaphore.h>
  #include <cerrno>
#endif

#include <stdexcept>


class Semaforo {
public:
    explicit Semaforo(unsigned int valorInicial) {
#if defined(_WIN32)
        handle = CreateSemaphore(nullptr, static_cast<LONG>(valorInicial), 0x7FFFFFFF, nullptr);
        if (handle == nullptr) throw std::runtime_error("No se pudo crear el semaforo (CreateSemaphore)");
#elif defined(__APPLE__)
        sem = dispatch_semaphore_create(static_cast<long>(valorInicial));
        if (sem == nullptr) throw std::runtime_error("No se pudo crear el semaforo (dispatch_semaphore_create)");
#else
        if (sem_init(&sem, 0, valorInicial) != 0) throw std::runtime_error("No se pudo crear el semaforo (sem_init)");
#endif
    }

// -------------------------------------------------------------------------------------------------------------------------------------------------

    // 1.4)LA HERRAMIENTA SEMAFORO
    // SEMÁFORO = contador protegido por el sistema operativo.
    ~Semaforo() {
#if defined(_WIN32)
        CloseHandle(handle);
#elif defined(__APPLE__)
        dispatch_release(sem);
#else
        sem_destroy(&sem);
#endif
    }

    //   esperar() (P): si vale 0, el thread se BLOQUEA sin gastar CPU.
    void esperar() {
#if defined(_WIN32)
        WaitForSingleObject(handle, INFINITE);
#elif defined(__APPLE__)
        dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);
#else
        
        while (sem_wait(&sem) != 0 && errno == EINTR) { }
#endif
    }

    //   senalar() (V): suma 1 y despierta a un thread bloqueado.
    void senalar() {
#if defined(_WIN32)
        ReleaseSemaphore(handle, 1, nullptr);
#elif defined(__APPLE__)
        dispatch_semaphore_signal(sem);
#else
        sem_post(&sem);
#endif
    }


    Semaforo(const Semaforo&) = delete;
    Semaforo& operator=(const Semaforo&) = delete;

private:
#if defined(_WIN32)
    HANDLE handle;
#elif defined(__APPLE__)
    dispatch_semaphore_t sem;
#else
    sem_t sem;
#endif
};

// Guardia RAII: entra con P al crearse y sale con V al destruirse
// Aunque ocurra una excepción, el semáforo siempre se libera
class SeccionCritica {
public:
    explicit SeccionCritica(Semaforo& s) : sem(s) { sem.esperar(); }
    ~SeccionCritica() { sem.senalar(); }
    SeccionCritica(const SeccionCritica&) = delete;
    SeccionCritica& operator=(const SeccionCritica&) = delete;
private:
    Semaforo& sem;
};

#endif 
