#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <limits>

// ---------------------------------------------------------------------
// Nodo del arbol / miembro de la familia
// ---------------------------------------------------------------------
struct Persona {
    int id = 0;
    std::string name;
    std::string last_name;
    char gender = 'H';
    int age = 0;
    int id_boss = 0;
    bool is_dead = false;
    bool in_jail = false;
    bool was_boss = false;
    bool is_boss = false;

    Persona* left = nullptr;   // primer sucesor
    Persona* right = nullptr;  // segundo sucesor
    Persona* parent = nullptr; // jefe al que responde
};

// ---------------------------------------------------------------------
// Cola propia (lista enlazada) para el recorrido BFS
// ---------------------------------------------------------------------
struct NodoCola {
    Persona* data;
    NodoCola* next;
    NodoCola(Persona* p) : data(p), next(nullptr) {}
};

class ColaPersonas {
    NodoCola* frente = nullptr;
    NodoCola* fin = nullptr;
public:
    ~ColaPersonas() { while (!vacia()) sacar(); }
    bool vacia() const { return frente == nullptr; }
    void meter(Persona* p) {
        NodoCola* nodo = new NodoCola(p);
        if (!fin) frente = fin = nodo;
        else { fin->next = nodo; fin = nodo; }
    }
    Persona* sacar() {
        if (vacia()) return nullptr;
        NodoCola* temp = frente;
        Persona* data = temp->data;
        frente = frente->next;
        if (!frente) fin = nullptr;
        delete temp;
        return data;
    }
};

// ---------------------------------------------------------------------
// Arbol de la familia
// ---------------------------------------------------------------------
class ArbolFamilia {
    Persona* raiz = nullptr;                 // patriarca original (id_boss == 0)
    std::map<int, Persona*> indice;          // id -> Persona*, solo para armar/editar el arbol

    // Busca en pre-orden, dentro del sub-arbol de "nodo" (incluyendolo),
    // el primer miembro vivo (y libre si incluirPresos es falso).
    Persona* buscarEnSubarbol(Persona* nodo, bool incluirPresos) const {
        if (!nodo) return nullptr;
        if (!nodo->is_dead && (incluirPresos || !nodo->in_jail)) return nodo;
        Persona* enc = buscarEnSubarbol(nodo->left, incluirPresos);
        if (enc) return enc;
        return buscarEnSubarbol(nodo->right, incluirPresos);
    }

    // Busca en TODO el arbol (BFS desde la raiz) el primer miembro vivo
    // (y libre si incluirPresos es falso).
    Persona* buscarEnTodaLaFamilia(bool incluirPresos) const {
        if (!raiz) return nullptr;
        ColaPersonas cola;
        cola.meter(raiz);
        while (!cola.vacia()) {
            Persona* actual = cola.sacar();
            if (!actual->is_dead && (incluirPresos || !actual->in_jail)) return actual;
            if (actual->left) cola.meter(actual->left);
            if (actual->right) cola.meter(actual->right);
        }
        return nullptr;
    }

    void imprimirPreOrden(Persona* nodo, int profundidad) const {
        if (!nodo) return;
        if (!nodo->is_dead) {
            for (int i = 0; i < profundidad; ++i) std::cout << "  ";
            std::cout << "- " << nodo->name << " " << nodo->last_name
                      << " (id:" << nodo->id << ", edad:" << nodo->age << ")";
            if (nodo->is_boss) std::cout << " [JEFE ACTUAL]";
            if (nodo->in_jail) std::cout << " [EN LA CARCEL]";
            if (nodo->was_boss) std::cout << " [EX-JEFE]";
            std::cout << "\n";
        }
        imprimirPreOrden(nodo->left, profundidad + 1);
        imprimirPreOrden(nodo->right, profundidad + 1);
    }

    void ascenderA(Persona* nuevoJefe, Persona* jefeAnterior) {
        if (jefeAnterior) { jefeAnterior->is_boss = false; jefeAnterior->was_boss = true; }
        nuevoJefe->is_boss = true;
    }

public:
    ~ArbolFamilia() {
        for (auto& par : indice) delete par.second;
    }

    bool cargarCSV(const std::string& ruta) {
        std::ifstream archivo(ruta);
        if (!archivo.is_open()) return false;

        std::string linea;
        std::getline(archivo, linea); // se descarta encabezado

        while (std::getline(archivo, linea)) {
            if (linea.empty()) continue;
            std::stringstream ss(linea);
            std::string token;
            Persona* p = new Persona();

            std::getline(ss, token, ','); p->id = std::stoi(token);
            std::getline(ss, token, ','); p->name = token;
            std::getline(ss, token, ','); p->last_name = token;
            std::getline(ss, token, ','); p->gender = token.empty() ? 'H' : token[0];
            std::getline(ss, token, ','); p->age = std::stoi(token);
            std::getline(ss, token, ','); p->id_boss = std::stoi(token);
            std::getline(ss, token, ','); p->is_dead = (token == "1");
            std::getline(ss, token, ','); p->in_jail = (token == "1");
            std::getline(ss, token, ','); p->was_boss = (token == "1");
            std::getline(ss, token, ','); p->is_boss = (token == "1");

            indice[p->id] = p;
        }
        archivo.close();

        // Segunda pasada: se enlazan padres e hijos usando id_boss.
        for (auto& par : indice) {
            Persona* p = par.second;
            if (p->id_boss == 0) { raiz = p; continue; }
            auto it = indice.find(p->id_boss);
            if (it == indice.end()) continue; // id_boss invalido, se ignora
            Persona* jefe = it->second;
            p->parent = jefe;
            if (!jefe->left) jefe->left = p;
            else if (!jefe->right) jefe->right = p;
            // si el jefe ya tiene dos sucesores, el resto se ignora (arbol binario)
        }
        return true;
    }

    bool guardarCSV(const std::string& ruta) const {
        std::ofstream archivo(ruta);
        if (!archivo.is_open()) return false;
        archivo << "id,name,last_name,gender,age,id_boss,is_dead,in_jail,was_boss,is_boss\n";
        for (const auto& par : indice) {
            Persona* p = par.second;
            archivo << p->id << "," << p->name << "," << p->last_name << ","
                    << p->gender << "," << p->age << "," << p->id_boss << ","
                    << (p->is_dead ? 1 : 0) << "," << (p->in_jail ? 1 : 0) << ","
                    << (p->was_boss ? 1 : 0) << "," << (p->is_boss ? 1 : 0) << "\n";
        }
        return true;
    }

    Persona* buscarPorId(int id) const {
        auto it = indice.find(id);
        return (it != indice.end()) ? it->second : nullptr;
    }

    Persona* jefeActual() const {
        for (const auto& par : indice) if (par.second->is_boss) return par.second;
        return nullptr;
    }

    void mostrarLineaSucesion() const {
        if (!raiz) { std::cout << "El arbol esta vacio.\n"; return; }
        std::cout << "\n===== LINEA DE SUCESION (miembros vivos) =====\n";
        imprimirPreOrden(raiz, 0);
        std::cout << "===============================================\n";
    }

    // Reglas de sucesion al morir un miembro (si era el jefe, se reasigna).
    void reportarMuerte(int id) {
        Persona* fallecido = buscarPorId(id);
        if (!fallecido) { std::cout << "No existe un miembro con ese id.\n"; return; }

        fallecido->is_dead = true;

        if (!fallecido->is_boss) {
            std::cout << fallecido->name << " " << fallecido->last_name
                      << " ha muerto. No era el jefe, no se requiere reasignacion.\n";
            return;
        }

        fallecido->is_boss = false;
        fallecido->was_boss = true;

        Persona* sucesor = nullptr;

        // 1) Primer sucesor vivo y libre dentro del propio arbol del fallecido.
        if (fallecido->left) sucesor = buscarEnSubarbol(fallecido->left, false);
        if (!sucesor && fallecido->right) sucesor = buscarEnSubarbol(fallecido->right, false);

        // 2) Si no hay, se sube por la familia buscando en cada rama "hermana".
        Persona* ancestro = fallecido->parent;
        Persona* vieneDe = fallecido;
        while (!sucesor && ancestro) {
            Persona* ramaHermana = (ancestro->left == vieneDe) ? ancestro->right : ancestro->left;
            if (ramaHermana) sucesor = buscarEnSubarbol(ramaHermana, false);
            vieneDe = ancestro;
            ancestro = ancestro->parent;
        }

        // 3) Si aun no se encuentra, se busca en toda la familia (BFS).
        if (!sucesor) sucesor = buscarEnTodaLaFamilia(false);

        // 4) Ultimo recurso: se permite elegir a alguien vivo aunque este preso.
        if (!sucesor) sucesor = buscarEnTodaLaFamilia(true);

        if (sucesor) {
            ascenderA(sucesor, fallecido);
            std::cout << "El jefe " << fallecido->name << " " << fallecido->last_name << " ha muerto.\n";
            std::cout << "Nuevo jefe de la familia: " << sucesor->name << " " << sucesor->last_name
                      << " (id:" << sucesor->id << ")";
            if (sucesor->in_jail) std::cout << " [asumido desde la carcel]";
            std::cout << "\n";
        } else {
            std::cout << "El jefe " << fallecido->name << " ha muerto y no hay ningun sucesor vivo en toda la familia.\n";
        }
    }

    // Jefe mayor de 70 anios o encarcelado: pasa el puesto al primer
    // sucesor libre y vivo de SU PROPIO arbol (sin cascada a otras ramas).
    void revisarRetiro(int id) {
        Persona* jefe = buscarPorId(id);
        if (!jefe || !jefe->is_boss) { std::cout << "Ese id no es el jefe actual.\n"; return; }

        if (jefe->age <= 70 && !jefe->in_jail) {
            std::cout << "No aplica retiro: " << jefe->name << " tiene " << jefe->age
                      << " anios y " << (jefe->in_jail ? "esta" : "no esta") << " en la carcel.\n";
            return;
        }

        Persona* sucesor = nullptr;
        if (jefe->left) sucesor = buscarEnSubarbol(jefe->left, false);
        if (!sucesor && jefe->right) sucesor = buscarEnSubarbol(jefe->right, false);

        if (sucesor) {
            ascenderA(sucesor, jefe);
            std::cout << jefe->name << " deja el puesto (edad/carcel). Nuevo jefe: "
                      << sucesor->name << " " << sucesor->last_name << "\n";
        } else {
            std::cout << jefe->name << " cumple la condicion de retiro, pero no hay sucesor libre en su propio arbol.\n";
        }
    }

    // Edita cualquier campo excepto id e id_boss.
    bool editar(int id, const std::string& campo, const std::string& valor) {
        Persona* p = buscarPorId(id);
        if (!p) return false;

        if (campo == "name") p->name = valor;
        else if (campo == "last_name") p->last_name = valor;
        else if (campo == "gender") {
            if (valor != "H" && valor != "M") return false;
            p->gender = valor[0];
        } else if (campo == "age") {
            try { p->age = std::stoi(valor); } catch (...) { return false; }
        } else if (campo == "is_dead") {
            if (valor != "0" && valor != "1") return false;
            p->is_dead = (valor == "1");
        } else if (campo == "in_jail") {
            if (valor != "0" && valor != "1") return false;
            p->in_jail = (valor == "1");
        } else if (campo == "was_boss") {
            if (valor != "0" && valor != "1") return false;
            p->was_boss = (valor == "1");
        } else if (campo == "is_boss") {
            if (valor != "0" && valor != "1") return false;
            p->is_boss = (valor == "1");
        } else {
            return false; // campo desconocido, o intento de tocar id / id_boss
        }
        return true;
    }
};

// ---------------------------------------------------------------------
// Menu principal
// ---------------------------------------------------------------------
void mostrarMenu() {
    std::cout << "\n================ CASA NOSTRA ================\n";
    std::cout << "1. Mostrar linea de sucesion (miembros vivos)\n";
    std::cout << "2. Reportar la muerte de un miembro\n";
    std::cout << "3. Revisar retiro del jefe (70+ anios / carcel)\n";
    std::cout << "4. Editar datos de un miembro\n";
    std::cout << "5. Ver quien es el jefe actual\n";
    std::cout << "6. Guardar cambios en el archivo CSV\n";
    std::cout << "0. Salir\n";
    std::cout << "==============================================\n";
    std::cout << "Seleccione una opcion: ";
}

void limpiarEntrada() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int main(int argc, char* argv[]) {
    ArbolFamilia familia;
    std::string ruta = (argc > 1) ? argv[1] : "data.csv";

    if (!familia.cargarCSV(ruta)) {
        std::cout << "No se pudo abrir '" << ruta << "'. Verifique que data.csv este junto al ejecutable.\n";
        return 1;
    }
    std::cout << "Familia cargada correctamente desde " << ruta << "\n";

    int opcion = -1;
    while (opcion != 0) {
        mostrarMenu();
        if (!(std::cin >> opcion)) { limpiarEntrada(); std::cout << "Entrada invalida.\n"; continue; }

        switch (opcion) {
            case 1:
                familia.mostrarLineaSucesion();
                break;
            case 2: {
                int id;
                std::cout << "Ingrese el id del miembro que murio: ";
                if (!(std::cin >> id)) { limpiarEntrada(); break; }
                familia.reportarMuerte(id);
                break;
            }
            case 3: {
                int id;
                std::cout << "Ingrese el id del jefe actual a revisar: ";
                if (!(std::cin >> id)) { limpiarEntrada(); break; }
                familia.revisarRetiro(id);
                break;
            }
            case 4: {
                int id;
                std::string campo, valor;
                std::cout << "Ingrese el id del miembro a editar: ";
                if (!(std::cin >> id)) { limpiarEntrada(); break; }
                std::cout << "Campo (name, last_name, gender, age, is_dead, in_jail, was_boss, is_boss): ";
                std::cin >> campo;
                std::cout << "Nuevo valor: ";
                std::cin >> valor;
                if (familia.editar(id, campo, valor)) std::cout << "Actualizado correctamente.\n";
                else std::cout << "No se pudo actualizar (revise id, campo o valor). id/id_boss no se pueden editar.\n";
                break;
            }
            case 5: {
                Persona* jefe = familia.jefeActual();
                if (jefe) std::cout << "Jefe actual: " << jefe->name << " " << jefe->last_name
                                     << " (id:" << jefe->id << ", edad:" << jefe->age << ")\n";
                else std::cout << "No hay ningun jefe asignado actualmente.\n";
                break;
            }
            case 6:
                if (familia.guardarCSV(ruta)) std::cout << "Cambios guardados en " << ruta << "\n";
                else std::cout << "Error al guardar.\n";
                break;
            case 0:
                std::cout << "Cerrando el programa. Arrivederci.\n";
                break;
            default:
                std::cout << "Opcion no valida.\n";
        }
    }
    return 0;
}
