#ifndef ENV_H
#define ENV_H

#include <string>

// Carga las variables del archivo .env
void cargarVariablesEntorno(const std::string& rutaEnv = ".env");

// Obtiene una variable de entorno desde el SO o del archivo .env cargado
std::string obtenerVariableEntorno(const std::string& clave, const std::string& valorPorDefecto = "");

// Si en la carpeta actual no existe 'rutaEnv' pero si existe junto al
// ejecutable, cambia la carpeta de trabajo a la del ejecutable. Asi el
// programa funciona aunque se lance desde otra carpeta (ej: ./SistOpe/sistope.exe).
// Devuelve la carpeta elegida en 'carpetaUsada' cuando hubo cambio.
bool ubicarCarpetaDelPrograma(const char* argv0, std::string& carpetaUsada, const std::string& rutaEnv = ".env");

// Convierte una ruta relativa a absoluta (respecto de la carpeta actual).
std::string rutaAbsoluta(const std::string& ruta);

#endif // ENV_H
