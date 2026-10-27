#ifndef CAMPOS_H
#define CAMPOS_H

#include <cstring>
#include <string>

// Copia un std::string a un campo char[] de tamano fijo.
// Siempre deja el campo terminado en '\0' y nunca escribe mas de 'tam' bytes.
inline void copiarCampo(char* destino, const std::string& origen, size_t tam) {
    std::memset(destino, 0, tam);
    std::strncpy(destino, origen.c_str(), tam - 1);
}

#endif // CAMPOS_H
