#ifndef ENV_H
#define ENV_H

#include <string>

// Carga las variables del archivo .env
void cargarVariablesEntorno(const std::string& rutaEnv = ".env");

// Obtiene una variable de entorno desde el SO o del archivo .env cargado
std::string obtenerVariableEntorno(const std::string& clave, const std::string& valorPorDefecto = "");

#endif // ENV_H
