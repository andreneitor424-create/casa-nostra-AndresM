#ifndef NODO_H
#define NODO_H

#include <string>

// Nodo del arbol binario: cada miembro tiene maximo dos sucesores (izq y der).
struct Nodo {
    int id;
    std::string nombre;
    std::string apellido;
    char genero;        // 'H' o 'M'
    int edad;
    int idJefe;         // 0 = no tiene jefe (raiz)
    bool muerto;
    bool enCarcel;
    bool fueJefe;
    bool esJefe;

    Nodo* izquierdo;
    Nodo* derecho;
    Nodo* padre;

    Nodo()
        : id(0), genero('H'), edad(0), idJefe(0),
          muerto(false), enCarcel(false), fueJefe(false), esJefe(false),
          izquierdo(nullptr), derecho(nullptr), padre(nullptr) {}
};

#endif
