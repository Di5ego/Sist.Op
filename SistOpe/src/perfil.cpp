#include "../include/perfil.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace std;

// Carga perfiles desde un archivo de texto hacia una estructura en memoria
bool cargarPerfilesDesdeArchivo(ListaPerfiles& lista, const string& rutaArchivo) {
    // Abre el archivo en modo lectura utilizando la ruta proporcionada
    ifstream file(rutaArchivo);
    
    // Verifica si ocurrió un error al abrir el archivo
    if (!file.is_open()) {
        // Lanza una advertencia indicando el fallo de apertura
        cerr << "Advertencia: No se pudo abrir el archivo de perfiles: " << rutaArchivo << endl;
        // Marca la memoria como cargada para evitar ciclos infinitos de recarga
        lista.cargadoEnMemoria = true;
        // Retorna false para indicar que la operación falló
        return false;
    }

    // Limpia la lista actual en memoria para evitar agregar datos duplicados
    lista.perfiles.clear();
    // Declara una variable para almacenar cada línea leída del archivo
    string line;
    
    // Lee el archivo línea por línea hasta llegar al final
    while (getline(file, line)) {
        // Ignora inmediatamente las líneas que estén completamente vacías
        if (line.empty()) continue;
        
        // Crea un stringstream para facilitar la separación de los datos de la línea
        stringstream ss(line);
        // Declara variables para almacenar el nombre y la cadena de opciones
        string nombre, opcionesStr;
        
        // Extrae el nombre hasta el delimitador ';' y el resto lo guarda en opcionesStr
        if (getline(ss, nombre, ';') && getline(ss, opcionesStr)) {
            // Verifica y elimina el salto de línea residual ('\r') común en archivos de Windows
            if (!opcionesStr.empty() && opcionesStr.back() == '\r') opcionesStr.pop_back();

            // Instancia un nuevo objeto Perfil
            Perfil p;
            // Le asigna el nombre extraído al perfil
            p.nombre = nombre;
            
            // Usa un segundo stringstream para procesar la cadena de opciones separada por comas
            stringstream ssOps(opcionesStr);
            // Declara un token temporal para cada opción individual
            string opToken;
            
            // Itera sobre los tokens de opciones separados por ','
            while (getline(ssOps, opToken, ',')) { 
                try {
                    // Convierte el token a entero y lo guarda en el vector de opciones
                    p.opciones.push_back(stoi(opToken)); 
                } catch (...) {
                    // Ignora silenciosamente los caracteres inválidos y continúa
                }
            }

            // Agrega el perfil ya construido a la lista principal
            lista.perfiles.push_back(p);
        }
    }
    
    // Cierra el archivo físico tras finalizar la lectura
    file.close();
    // Actualiza el estado indicando que la lista ya está cargada en memoria
    lista.cargadoEnMemoria = true;
    // Retorna true confirmando el éxito de la carga
    return true;
}

// Guarda un nuevo perfil de usuario en memoria y al final del archivo físico
bool guardarPerfil(ListaPerfiles& lista, const Perfil& nuevoPerfil, const string& rutaArchivo) {
    // Añade el nuevo perfil al final de la lista en memoria
    lista.perfiles.push_back(nuevoPerfil);

    // Abre el archivo en modo "append" (ios::app) para agregar contenido sin borrar lo existente
    ofstream file(rutaArchivo, ios::app);
    
    // Valida que el archivo se haya abierto exitosamente
    if (!file.is_open()) {
        // Emite un error por consola si falla la apertura
        cerr << "Error: No se pudo abrir el archivo para guardar el perfil." << endl;
        // Retorna false indicando el error
        return false;
    }

    // Escribe el nombre del nuevo perfil seguido por un punto y coma
    file << nuevoPerfil.nombre << ";";
    
    // Recorre el vector de opciones numéricas asociadas al perfil
    for (size_t i = 0; i < nuevoPerfil.opciones.size(); ++i) {
        // Escribe cada opción numérica en el archivo
        file << nuevoPerfil.opciones[i];
        // Añade una coma si no es la última opción de la lista
        if (i + 1 < nuevoPerfil.opciones.size()) {
            file << ",";
        }
    }
    // Inserta un salto de línea al finalizar la escritura de este perfil
    file << "\n";
    
    // Cierra el flujo del archivo
    file.close();
    // Retorna true para confirmar que se guardó correctamente
    return true;
}

// Muestra en consola una tabla con todos los perfiles registrados y sus opciones
void listarPerfiles(ListaPerfiles& lista, const string& rutaArchivo) {
    // Comprueba si la lista está cargada en memoria; de lo contrario, la carga primero
    if (!lista.cargadoEnMemoria) {
        cargarPerfilesDesdeArchivo(lista, rutaArchivo);
    }

    // Imprime el encabezado decorativo superior de la tabla
    cout << "\n------------------------------------------------------------\n";
    cout << "                    LISTA DE PERFILES                       \n";
    cout << "------------------------------------------------------------\n";
    
    // Imprime los títulos de las columnas con formato alineado a la izquierda y ancho fijo
    cout << left << setw(20) << "Nombre Perfil"
         << "Opciones Permitiadas (IDs)\n";
    cout << "------------------------------------------------------------\n";
    
    // Verifica si el vector de perfiles se encuentra vacío
    if (lista.perfiles.empty()) {
        // Muestra un mensaje notificando que no existen registros
        cout << "(No hay perfiles registrados)\n";
    } else {
        // Itera a través de cada perfil almacenado en la lista
        for (const auto& p : lista.perfiles) {
            // Imprime el nombre del perfil respetando el espaciado de la columna
            cout << left << setw(20) << p.nombre;
            
            // Inicializa una cadena vacía para ir concatenando las opciones
            string opsStr = "";
            
            // Itera por cada opción guardada en el perfil actual
            for (size_t i = 0; i < p.opciones.size(); ++i) {
                // Convierte el número de la opción a texto y lo concatena
                opsStr += to_string(p.opciones[i]);
        
                // Agrega una coma y un espacio, excepto para la última opción
                if (i + 1 < p.opciones.size()) opsStr += ", ";
            }
            // Imprime la cadena final de opciones seguida de un salto de línea
            cout << opsStr << "\n";
        }
    }
    // Cierra visualmente la tabla inferior
    cout << "------------------------------------------------------------\n";
}

// Busca y elimina un perfil específico por nombre, actualizando memoria y archivo
bool eliminarPerfilPorNombre(ListaPerfiles& lista, const string& nombrePerfil, const string& rutaArchivo) {
    // Asegura que los perfiles estén cargados en memoria antes de intentar buscar
    if (!lista.cargadoEnMemoria) {
        cargarPerfilesDesdeArchivo(lista, rutaArchivo);
    }
    
    // Busca el perfil cuyo nombre coincida exactamente usando find_if y una expresión lambda
    auto it = find_if(lista.perfiles.begin(), lista.perfiles.end(), [&nombrePerfil](const Perfil& p) {
        return p.nombre == nombrePerfil;
    });
    
    // Si el iterador llega al final, significa que no se encontró el perfil
    if (it == lista.perfiles.end()) {
        // Imprime un mensaje informando que no existe
        cout << "Error: No se encontró ningún perfil llamado '" << nombrePerfil << "'.\n";
        // Retorna false indicando que no se pudo borrar
        return false;
    }

    // Usa erase() para borrar el elemento de la estructura en memoria
    lista.perfiles.erase(it);

    // Abre el archivo en modo truncado (ios::trunc) para sobreescribir todo su contenido
    ofstream file(rutaArchivo, ios::trunc);
    
    // Valida que la apertura en modo escritura fuera exitosa
    if (!file.is_open()) {
        // Si falla, emite un error indicando que no se puede actualizar el archivo
        cerr << "Error: No se pudo abrir el archivo para actualizar la lista de perfiles" << endl;
        // Retorna false
        return false;
    }
    
    // Recorre la lista de perfiles actualizada que quedó en memoria
    for (const auto& p : lista.perfiles) { 
        // Escribe el nombre del perfil y el delimitador ';'
        file << p.nombre << ";";
        
        // Recorre y escribe cada opción numérica del perfil
        for (size_t i = 0; i < p.opciones.size(); ++i) {
            // Escribe el número de la opción
            file << p.opciones[i];
            // Agrega una coma si no es la última opción de este perfil
            if (i + 1 < p.opciones.size()) file << ",";
        }
        // Inserta un salto de línea para pasar al siguiente perfil
        file << "\n";
    }
    
    // Cierra el archivo físico tras volcar todos los datos
    file.close();
    // Retorna true confirmando que el perfil fue eliminado con éxito
    return true;
}

// Verifica de forma rápida si un perfil con el nombre especificado ya está registrado
bool existePerfil(const ListaPerfiles& lista, const string& nombrePerfil) {
    // Itera secuencialmente sobre la lista de perfiles en memoria
    for (const auto& p : lista.perfiles) {
        // Compara el nombre de cada perfil iterado con el nombre buscado; si coincide, retorna true
        if (p.nombre == nombrePerfil) return true;
    }
    // Si el ciclo finaliza sin encontrar coincidencias, retorna false
    return false;
}