#ifndef CONTEO_H
#define CONTEO_H

#include <string>

// Estructura para almacenar el resumen del conteo de caracteres y palabras
struct ResumenConteo {
    int vocales = 0;
    int consonantes = 0;
    int especiales = 0;
    int palabras = 0;
    bool exito = false;
    std::string mensajeError = "";
};

ResumenConteo contarTextoArchivo(const std::string& rutaArchivo);
void mostrarInterfazConteo(const std::string& rutaArchivo, const std::string& tituloMenu);

#endif // CONTEO_H
