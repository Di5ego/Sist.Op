CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread -Iinclude
LDLIBS   =

# En Windows (MinGW / MSYS2) los sockets requieren la biblioteca Winsock2
ifeq ($(OS),Windows_NT)
  LDLIBS += -lws2_32
  CXXFLAGS += -static-libgcc -static-libstdc++
endif

# ----------------------------------------------------------------------------
# Cada entrega es un programa independiente. El MENU PRINCIPAL (sistope.exe)
# ejecuta los demas mediante llamadas a sistema (ver include/externo.h).
# ----------------------------------------------------------------------------

# ---- Entrega 1: Administracion de usuarios y perfiles ----
SRCS_ADMIN = src/admin_main.cpp src/admin_usuarios.cpp src/usuario.cpp src/perfil.cpp src/env.cpp
TARGET_ADMIN = admin_usuarios.exe

# ---- Entrega 2: MENU PRINCIPAL (ampliado en la Entrega 3) ----
SRCS_MAIN = src/main.cpp src/usuario.cpp src/perfil.cpp src/menu.cpp src/env.cpp src/cli.cpp \
            src/conteo.cpp src/utilidades.cpp src/externo.cpp \
            src/juego_menu.cpp src/juego_config.cpp
TARGET_MAIN = sistope.exe

# ---- Entrega 2: Multiplicador de matrices ----
SRCS_MULTI = src/multi_main.cpp src/matriz.cpp
TARGET_MULTI = multi.exe

# ---- Entrega 3 - Problema 1: Indice invertido concurrente ----
SRCS_INDEX = src/indice_main.cpp src/indice_invertido.cpp src/env.cpp
TARGET_INDEX = indice_invertido.exe

# ---- Entrega 3 - Problema 2: Servidor del juego ----
SRCS_SERVER = src/juego_servidor.cpp src/juego_config.cpp src/red.cpp src/env.cpp
TARGET_SERVER = juego_server.exe

# ---- Entrega 3 - Problema 2: Cliente del juego ----
SRCS_CLIENT = src/juego_cliente_main.cpp src/juego_cliente.cpp src/juego_config.cpp src/red.cpp src/env.cpp
TARGET_CLIENT = juego_cliente.exe

TARGETS = $(TARGET_ADMIN) $(TARGET_MAIN) $(TARGET_MULTI) $(TARGET_INDEX) $(TARGET_SERVER) $(TARGET_CLIENT)
HEADERS = $(wildcard include/*.h)

all: $(TARGETS)

$(TARGET_ADMIN): $(SRCS_ADMIN) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRCS_ADMIN) -o $@ $(LDLIBS)

$(TARGET_MAIN): $(SRCS_MAIN) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRCS_MAIN) -o $@ $(LDLIBS)

$(TARGET_MULTI): $(SRCS_MULTI) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRCS_MULTI) -o $@ $(LDLIBS)

$(TARGET_INDEX): $(SRCS_INDEX) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRCS_INDEX) -o $@ $(LDLIBS)

$(TARGET_SERVER): $(SRCS_SERVER) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRCS_SERVER) -o $@ $(LDLIBS)

$(TARGET_CLIENT): $(SRCS_CLIENT) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRCS_CLIENT) -o $@ $(LDLIBS)

clean:
	rm -f $(TARGETS) src/*.o

.PHONY: all clean
