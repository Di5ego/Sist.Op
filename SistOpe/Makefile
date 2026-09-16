CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude

SRCS_MAIN = src/main.cpp src/usuario.cpp src/perfil.cpp src/menu.cpp src/env.cpp src/cli.cpp src/conteo.cpp src/utilidades.cpp src/matriz.cpp
TARGET_MAIN = sistope.exe

SRCS_MULTI = src/multi_main.cpp src/matriz.cpp
TARGET_MULTI = multi.exe

all: $(TARGET_MAIN) $(TARGET_MULTI)

$(TARGET_MAIN): $(SRCS_MAIN)
	$(CXX) $(CXXFLAGS) $(SRCS_MAIN) -o $(TARGET_MAIN)

$(TARGET_MULTI): $(SRCS_MULTI)
	$(CXX) $(CXXFLAGS) $(SRCS_MULTI) -o $(TARGET_MULTI)

clean:
	rm -f $(TARGET_MAIN) $(TARGET_MULTI) src/*.o *.exe

.PHONY: all clean
