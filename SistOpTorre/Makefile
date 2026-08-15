CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude

SRCS = src/main.cpp src/usuario.cpp src/perfil.cpp src/menu.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = sistope

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET) src/*.o *.exe

.PHONY: all clean
