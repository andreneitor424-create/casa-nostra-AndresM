#include <iostream>
#include <limits>
#include <string>

#include "arbol_familia.h"

static int leerEntero(const std::string& mensaje) {
    int valor;
    while (true) {
        std::cout << mensaje;
        if (std::cin >> valor) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return valor;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Entrada invalida, intente de nuevo.\n";
    }
}

static bool leerBool(const std::string& mensaje) {
    while (true) {
        int valor = leerEntero(mensaje + " (1 = si, 0 = no): ");
        if (valor == 0 || valor == 1) return valor == 1;
        std::cout << "Solo se acepta 0 o 1.\n";
    }
}

static void modificarMiembro(ArbolFamilia& arbol) {
    int id = leerEntero("Id del miembro a modificar: ");
    Nodo* nodo = arbol.buscarPorId(id);
    if (!nodo) {
        std::cout << "No existe un miembro con ese id.\n";
        return;
    }

    std::cout << "\nModificando a " << nodo->nombre << " " << nodo->apellido
              << " (el id y el id del jefe no se pueden cambiar).\n";
    std::cout << "1. Nombre        (" << nodo->nombre << ")\n";
    std::cout << "2. Apellido      (" << nodo->apellido << ")\n";
    std::cout << "3. Genero        (" << nodo->genero << ")\n";
    std::cout << "4. Edad          (" << nodo->edad << ")\n";
    std::cout << "5. Muerto        (" << (nodo->muerto ? 1 : 0) << ")\n";
    std::cout << "6. En la carcel  (" << (nodo->enCarcel ? 1 : 0) << ")\n";
    std::cout << "7. Fue jefe      (" << (nodo->fueJefe ? 1 : 0) << ")\n";
    std::cout << "8. Es jefe       (" << (nodo->esJefe ? 1 : 0) << ")\n";

    int opcion = leerEntero("Campo a cambiar: ");
    switch (opcion) {
        case 1:
            std::cout << "Nuevo nombre: ";
            std::getline(std::cin, nodo->nombre);
            break;
        case 2:
            std::cout << "Nuevo apellido: ";
            std::getline(std::cin, nodo->apellido);
            break;
        case 3: {
            std::string genero;
            while (genero != "H" && genero != "M") {
                std::cout << "Nuevo genero (H o M): ";
                std::getline(std::cin, genero);
            }
            nodo->genero = genero[0];
            break;
        }
        case 4:
            nodo->edad = leerEntero("Nueva edad: ");
            break;
        case 5:
            nodo->muerto = leerBool("Esta muerto?");
            break;
        case 6:
            nodo->enCarcel = leerBool("Esta en la carcel?");
            break;
        case 7:
            nodo->fueJefe = leerBool("Fue jefe?");
            break;
        case 8:
            nodo->esJefe = leerBool("Es jefe?");
            break;
        default:
            std::cout << "Opcion invalida.\n";
            return;
    }

    std::cout << "Dato actualizado.\n";
    // Cualquier cambio puede afectar la jefatura (muerte, carcel, edad).
    arbol.verificarJefe();
}

int main(int argc, char* argv[]) {
    std::string ruta = "familia.csv";
    if (argc > 1) ruta = argv[1];

    ArbolFamilia arbol;
    if (!arbol.cargarDesdeCsv(ruta)) {
        std::cout << "No se pudo cargar el arbol. Verifique el archivo de datos.\n";
        return 1;
    }
    std::cout << "Datos cargados desde " << ruta << ".\n";
    arbol.verificarJefe();

    bool seguir = true;
    while (seguir) {
        std::cout << "\n========= LA FAMILIA =========\n";
        std::cout << "1. Mostrar linea de sucesion (vivos)\n";
        std::cout << "2. Mostrar jefe actual\n";
        std::cout << "3. Mostrar arbol completo\n";
        std::cout << "4. Modificar datos de un miembro\n";
        std::cout << "5. Guardar cambios en el CSV\n";
        std::cout << "6. Salir\n";

        int opcion = leerEntero("Opcion: ");
        switch (opcion) {
            case 1:
                arbol.mostrarLineaSucesion();
                break;
            case 2:
                arbol.mostrarJefe();
                break;
            case 3:
                arbol.mostrarArbol();
                break;
            case 4:
                modificarMiembro(arbol);
                break;
            case 5:
                if (arbol.guardarEnCsv(ruta))
                    std::cout << "Cambios guardados en " << ruta << ".\n";
                break;
            case 6:
                seguir = false;
                break;
            default:
                std::cout << "Opcion invalida.\n";
        }
    }

    std::cout << "Hasta luego.\n";
    return 0;
}
