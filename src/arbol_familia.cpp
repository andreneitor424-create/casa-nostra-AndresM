#include "arbol_familia.h"

#include <fstream>
#include <iostream>
#include <sstream>

ArbolFamilia::ArbolFamilia() : raiz(nullptr) {}

ArbolFamilia::~ArbolFamilia() {
    destruir(raiz);
}

void ArbolFamilia::destruir(Nodo* nodo) {
    if (!nodo) return;
    destruir(nodo->izquierdo);
    destruir(nodo->derecho);
    delete nodo;
}

// ---------- Carga y guardado ----------

static bool aBool(const std::string& s) {
    return s == "1";
}

bool ArbolFamilia::cargarDesdeCsv(const std::string& ruta) {
    std::ifstream archivo(ruta.c_str());
    if (!archivo.is_open()) {
        std::cout << "No se pudo abrir el archivo: " << ruta << "\n";
        return false;
    }

    std::string linea;
    std::getline(archivo, linea); // cabecera

    // Lista enlazada de nodos pendientes de insertar (no se permiten vectores).
    ElementoLista* pendientes = nullptr;
    ElementoLista* ultimo = nullptr;

    while (std::getline(archivo, linea)) {
        if (linea.empty()) continue;
        std::stringstream ss(linea);
        std::string campo;
        Nodo* nodo = new Nodo();

        std::getline(ss, campo, ','); nodo->id = std::stoi(campo);
        std::getline(ss, nodo->nombre, ',');
        std::getline(ss, nodo->apellido, ',');
        std::getline(ss, campo, ','); nodo->genero = campo.empty() ? 'H' : campo[0];
        std::getline(ss, campo, ','); nodo->edad = std::stoi(campo);
        std::getline(ss, campo, ','); nodo->idJefe = campo.empty() ? 0 : std::stoi(campo);
        std::getline(ss, campo, ','); nodo->muerto = aBool(campo);
        std::getline(ss, campo, ','); nodo->enCarcel = aBool(campo);
        std::getline(ss, campo, ','); nodo->fueJefe = aBool(campo);
        std::getline(ss, campo, ','); nodo->esJefe = aBool(campo);

        ElementoLista* elemento = new ElementoLista(nodo);
        if (!pendientes) {
            pendientes = elemento;
            ultimo = elemento;
        } else {
            ultimo->siguiente = elemento;
            ultimo = elemento;
        }
    }
    archivo.close();

    // Insertar hasta que no quede ninguno (el padre puede aparecer despues en el archivo).
    bool huboCambio = true;
    while (pendientes && huboCambio) {
        huboCambio = false;
        ElementoLista* anterior = nullptr;
        ElementoLista* actual = pendientes;
        while (actual) {
            Nodo* nodo = actual->dato;
            bool insertado = false;
            if (nodo->idJefe == 0 && !raiz) {
                raiz = nodo;
                insertado = true;
            } else if (nodo->idJefe != 0) {
                Nodo* padre = buscarPorId(nodo->idJefe);
                if (padre) {
                    nodo->padre = padre;
                    if (!padre->izquierdo) padre->izquierdo = nodo;
                    else if (!padre->derecho) padre->derecho = nodo;
                    else {
                        std::cout << "Aviso: el miembro " << nodo->id
                                  << " no cabe, su jefe ya tiene dos sucesores.\n";
                        delete nodo;
                    }
                    insertado = true;
                }
            }

            if (insertado) {
                huboCambio = true;
                ElementoLista* borrar = actual;
                if (anterior) anterior->siguiente = actual->siguiente;
                else pendientes = actual->siguiente;
                actual = actual->siguiente;
                delete borrar;
            } else {
                anterior = actual;
                actual = actual->siguiente;
            }
        }
    }

    // Limpiar huerfanos que nunca encontraron a su jefe.
    while (pendientes) {
        std::cout << "Aviso: el miembro " << pendientes->dato->id
                  << " quedo fuera, no se encontro a su jefe.\n";
        delete pendientes->dato;
        ElementoLista* borrar = pendientes;
        pendientes = pendientes->siguiente;
        delete borrar;
    }

    return raiz != nullptr;
}

void ArbolFamilia::guardarRec(Nodo* nodo, std::ofstream& salida) const {
    if (!nodo) return;
    salida << nodo->id << ',' << nodo->nombre << ',' << nodo->apellido << ','
           << nodo->genero << ',' << nodo->edad << ',' << nodo->idJefe << ','
           << (nodo->muerto ? 1 : 0) << ',' << (nodo->enCarcel ? 1 : 0) << ','
           << (nodo->fueJefe ? 1 : 0) << ',' << (nodo->esJefe ? 1 : 0) << "\n";
    guardarRec(nodo->izquierdo, salida);
    guardarRec(nodo->derecho, salida);
}

bool ArbolFamilia::guardarEnCsv(const std::string& ruta) const {
    std::ofstream salida(ruta.c_str());
    if (!salida.is_open()) {
        std::cout << "No se pudo escribir el archivo: " << ruta << "\n";
        return false;
    }
    salida << "id,name,last_name,gender,age,id_boss,is_dead,in_jail,was_boss,is_boss\n";
    guardarRec(raiz, salida);
    return true;
}

// ---------- Busquedas ----------

Nodo* ArbolFamilia::buscarPorId(int id) const {
    return buscarPorIdRec(raiz, id);
}

Nodo* ArbolFamilia::buscarPorIdRec(Nodo* nodo, int id) const {
    if (!nodo) return nullptr;
    if (nodo->id == id) return nodo;
    Nodo* encontrado = buscarPorIdRec(nodo->izquierdo, id);
    if (encontrado) return encontrado;
    return buscarPorIdRec(nodo->derecho, id);
}

Nodo* ArbolFamilia::buscarJefeActual(Nodo* nodo) const {
    if (!nodo) return nullptr;
    if (nodo->esJefe) return nodo;
    Nodo* encontrado = buscarJefeActual(nodo->izquierdo);
    if (encontrado) return encontrado;
    return buscarJefeActual(nodo->derecho);
}

Nodo* ArbolFamilia::primerVivoLibre(Nodo* nodo, Nodo* excluido) const {
    if (!nodo) return nullptr;
    if (nodo != excluido && !nodo->muerto && !nodo->enCarcel) return nodo;
    Nodo* encontrado = primerVivoLibre(nodo->izquierdo, excluido);
    if (encontrado) return encontrado;
    return primerVivoLibre(nodo->derecho, excluido);
}

// BFS por cercania desde el origen, recorriendo padre e hijos.
// permitirCarcel = true se usa como ultimo recurso (todos los libres murieron).
Nodo* ArbolFamilia::masCercano(Nodo* origen, bool permitirCarcel) const {
    if (!origen) return nullptr;

    ElementoLista* frente = new ElementoLista(origen);
    ElementoLista* final = frente;
    ElementoLista* visitados = nullptr;
    Nodo* resultado = nullptr;

    while (frente && !resultado) {
        Nodo* actual = frente->dato;
        ElementoLista* usado = frente;
        frente = frente->siguiente;
        if (!frente) final = nullptr;

        usado->siguiente = visitados;
        visitados = usado;

        if (actual != origen && !actual->muerto &&
            (permitirCarcel || !actual->enCarcel)) {
            resultado = actual;
            break;
        }

        Nodo* vecinos[3] = { actual->padre, actual->izquierdo, actual->derecho };
        for (int i = 0; i < 3; ++i) {
            Nodo* vecino = vecinos[i];
            if (!vecino) continue;
            bool yaVisto = false;
            for (ElementoLista* v = visitados; v && !yaVisto; v = v->siguiente)
                if (v->dato == vecino) yaVisto = true;
            for (ElementoLista* v = frente; v && !yaVisto; v = v->siguiente)
                if (v->dato == vecino) yaVisto = true;
            if (yaVisto) continue;

            ElementoLista* elemento = new ElementoLista(vecino);
            if (!frente) { frente = elemento; final = elemento; }
            else { final->siguiente = elemento; final = elemento; }
        }
    }

    while (frente) {
        ElementoLista* borrar = frente;
        frente = frente->siguiente;
        delete borrar;
    }
    while (visitados) {
        ElementoLista* borrar = visitados;
        visitados = visitados->siguiente;
        delete borrar;
    }
    return resultado;
}

// ---------- Reglas de sucesion ----------

Nodo* ArbolFamilia::elegirSucesor(Nodo* jefe) const {
    // 1. Primer sucesor vivo y libre dentro de su propio arbol.
    Nodo* candidato = primerVivoLibre(jefe, jefe);
    if (candidato) return candidato;

    // 2. Subir por los ancestros: buscar en el arbol del companero sucesor
    //    (el otro hijo del jefe de su jefe). Si el companero esta vivo, libre
    //    y sin sucesores, el mismo toma el puesto (el preorden lo cubre).
    Nodo* actual = jefe;
    while (actual->padre) {
        Nodo* padre = actual->padre;
        Nodo* companero = (padre->izquierdo == actual) ? padre->derecho : padre->izquierdo;
        candidato = primerVivoLibre(companero, nullptr);
        if (candidato) return candidato;
        actual = padre;
    }

    // 3. Jefe mas cercano con sucesores libres: BFS por cercania sobre todo
    //    el arbol y tomar el primer vivo y libre.
    candidato = masCercano(jefe, false);
    if (candidato) return candidato;

    // 4. Ultimo recurso: todos los libres murieron, se aceptan vivos en la
    //    carcel, empezando por el mas cercano al jefe actual.
    return masCercano(jefe, true);
}

void ArbolFamilia::asignarJefe(Nodo* anterior, Nodo* nuevo) {
    if (anterior) {
        anterior->esJefe = false;
        anterior->fueJefe = true;
    }
    if (nuevo) {
        nuevo->esJefe = true;
        std::cout << ">> Nuevo jefe de la familia: " << nuevo->nombre << " "
                  << nuevo->apellido << " (id " << nuevo->id << ")\n";
    }
}

void ArbolFamilia::verificarJefe() {
    Nodo* jefe = buscarJefeActual(raiz);

    if (!jefe) {
        // Nadie marcado como jefe: se aplica la busqueda desde la raiz.
        Nodo* nuevo = primerVivoLibre(raiz, nullptr);
        if (!nuevo && raiz) nuevo = masCercano(raiz, true);
        if (!nuevo && raiz && !raiz->muerto) nuevo = raiz;
        if (nuevo) asignarJefe(nullptr, nuevo);
        else std::cout << ">> No queda nadie vivo para ser jefe.\n";
        return;
    }

    // Repetir por si el sucesor elegido tambien resulta invalido.
    while (jefe && (jefe->muerto || jefe->enCarcel || jefe->edad > 70)) {
        if (jefe->muerto)
            std::cout << ">> El jefe " << jefe->nombre << " " << jefe->apellido << " murio.\n";
        else if (jefe->enCarcel)
            std::cout << ">> El jefe " << jefe->nombre << " " << jefe->apellido << " fue a prision.\n";
        else
            std::cout << ">> El jefe " << jefe->nombre << " " << jefe->apellido << " paso de los 70 anios.\n";

        Nodo* nuevo = elegirSucesor(jefe);
        asignarJefe(jefe, nuevo);
        if (!nuevo) {
            std::cout << ">> No se encontro ningun sucesor vivo.\n";
            return;
        }
        jefe = nuevo;
    }
}

// ---------- Mostrar ----------

void ArbolFamilia::sucesionRec(Nodo* nodo, int& contador) const {
    if (!nodo) return;
    if (!nodo->muerto) {
        std::cout << "  " << contador++ << ". " << nodo->nombre << " " << nodo->apellido
                  << " (id " << nodo->id << ", " << nodo->edad << " anios)";
        if (nodo->esJefe) std::cout << " [JEFE ACTUAL]";
        if (nodo->enCarcel) std::cout << " [EN LA CARCEL]";
        std::cout << "\n";
    }
    sucesionRec(nodo->izquierdo, contador);
    sucesionRec(nodo->derecho, contador);
}

void ArbolFamilia::mostrarLineaSucesion() const {
    std::cout << "\n--- Linea de sucesion actual (solo vivos) ---\n";
    int contador = 1;
    sucesionRec(raiz, contador);
    if (contador == 1) std::cout << "  No queda nadie vivo.\n";
}

void ArbolFamilia::arbolRec(Nodo* nodo, int nivel) const {
    if (!nodo) return;
    for (int i = 0; i < nivel; ++i) std::cout << "   ";
    std::cout << "- " << nodo->nombre << " " << nodo->apellido
              << " (id " << nodo->id << ", " << nodo->genero << ", "
              << nodo->edad << " anios)";
    if (nodo->esJefe) std::cout << " [JEFE]";
    if (nodo->fueJefe) std::cout << " [EX JEFE]";
    if (nodo->muerto) std::cout << " [MUERTO]";
    if (nodo->enCarcel) std::cout << " [CARCEL]";
    std::cout << "\n";
    arbolRec(nodo->izquierdo, nivel + 1);
    arbolRec(nodo->derecho, nivel + 1);
}

void ArbolFamilia::mostrarArbol() const {
    std::cout << "\n--- Arbol completo de la familia ---\n";
    arbolRec(raiz, 0);
}

void ArbolFamilia::mostrarJefe() const {
    Nodo* jefe = buscarJefeActual(raiz);
    if (jefe)
        std::cout << "\nJefe actual: " << jefe->nombre << " " << jefe->apellido
                  << " (id " << jefe->id << ", " << jefe->edad << " anios)\n";
    else
        std::cout << "\nLa familia no tiene jefe en este momento.\n";
}
