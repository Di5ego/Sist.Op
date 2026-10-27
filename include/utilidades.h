#ifndef UTILIDADES_H
#define UTILIDADES_H

#include <string>

// Revisa si una cadena es palindromo (ignorando mayusculas, espacios y puntuacion)
bool esPalindromo(const std::string& texto);

// Calcula f(x) = x*x + 2x + 8 para numeros reales
double calcularFuncion(double x);

// Interfaz para la opcion 4 (Es palindromo?)
void menuPalindromo();

// Interfaz para la opcion 5 (Calcular f(x))
void menuCalcularFuncion();

#endif // UTILIDADES_H
