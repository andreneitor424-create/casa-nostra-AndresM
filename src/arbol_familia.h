#ifndef ARBOL_FAMILIA_H
#define ARBOL_FAMILIA_H

#include <string>
#include "nodo.h"

// Lista enlazada simple usada como cola/lista auxiliar (no se permiten vectores).
struct ElementoLista {
    Nodo* dato;
    ElementoLista* siguiente;
    ElementoLista(Nodo* n) : dato(n), siguiente(nullptr) {}
};

class ArbolFamilia {
public:
    ArbolFamilia();
    ~ArbolFamilia();

    bool cargarDesdeCsv(const std::string& ruta);
    bool guardarEnCsv(const std::string& ruta) const;

    void mostrarLineaSucesion() const;   // solo vivos, en orden de sucesion
    void mostrarArbol() const;
    void mostrarJefe() const;

    Nodo* buscarPorId(int id) const;

    // Revisa al jefe actual y reasigna si murio, paso de 70 o esta en la carcel.
    // Repite hasta que el jefe sea valido o no quede nadie.
    void verificarJefe();

private:
    Nodo* raiz;

    void destruir(Nodo* nodo);
    void insertarEnArbol(Nodo* nuevo);
    Nodo* buscarPorIdRec(Nodo* nodo, int id) const;
    Nodo* buscarJefeActual(Nodo* nodo) const;

    // Primer nodo vivo y libre en preorden dentro del subarbol (puede excluir la raiz del subarbol).
    Nodo* primerVivoLibre(Nodo* nodo, Nodo* excluido) const;

    // Busqueda por cercania (BFS con padre e hijos) desde un nodo.
    Nodo* masCercano(Nodo* origen, bool permitirCarcel) const;

    Nodo* elegirSucesor(Nodo* jefe) const;
    void asignarJefe(Nodo* anterior, Nodo* nuevo);

    void sucesionRec(Nodo* nodo, int& contador) const;
    void arbolRec(Nodo* nodo, int nivel) const;
    void guardarRec(Nodo* nodo, std::ofstream& salida) const;
};

#endif
