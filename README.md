# La Familia — Sucesión de la mafia

Prototipo en C++ que administra la línea de sucesión de una familia de la mafia
usando un **árbol binario** (cada miembro tiene como máximo dos sucesores).
No se utilizan vectores: la carga y las búsquedas por cercanía usan listas
enlazadas propias.

## Estructura del proyecto

```
├── bin/          # Ejecutable y datos de prueba (familia.csv)
├── src/          # Código fuente
│   ├── main.cpp
│   ├── arbol_familia.h
│   ├── arbol_familia.cpp
│   └── nodo.h
└── README.md
```

## Formato del CSV

```
id,name,last_name,gender,age,id_boss,is_dead,in_jail,was_boss,is_boss
```

- `gender`: `H` (hombre) o `M` (mujer).
- `id_boss`: id del jefe directo; `0` para el jefe raíz.
- `is_dead`, `in_jail`, `was_boss`, `is_boss`: `1` afirmativo, `0` negativo.

## Compilación

Desde la raíz del proyecto:

```bash
g++ -std=c++11 -Wall src/main.cpp src/arbol_familia.cpp -o bin/familia
```

## Ejecución

```bash
cd bin
./familia            # usa familia.csv por defecto
./familia otros.csv  # o un archivo distinto
```

El programa se mantiene en ejecución mostrando un menú hasta que el usuario
elija la opción de salir.

## Funcionalidades

1. **Carga desde CSV**: convierte los datos en un árbol binario. El orden de
   las filas no importa: los nodos esperan en una lista enlazada hasta que su
   jefe aparece en el árbol.
2. **Línea de sucesión**: recorrido en preorden mostrando solo a los vivos.
3. **Asignación automática de jefe** cuando el jefe muere, pasa de los 70
   años o va a prisión:
   - Primero se busca el primer sucesor vivo y libre en el árbol del jefe
     (preorden).
   - Si no hay, se sube por los ancestros y se busca en el árbol del
     compañero sucesor (el otro hijo del jefe de su jefe). Si el compañero
     está vivo, libre y sin sucesores, él toma el puesto.
   - Si tampoco hay, se busca por cercanía (BFS sobre el árbol) al primer
     sucesor vivo y libre del jefe más cercano.
   - Último recurso: si todos los libres murieron, se aceptan miembros vivos
     en la cárcel, empezando por el más cercano al jefe actual.
   - La verificación se repite por si el nuevo jefe también resulta inválido.
4. **Modificación de datos**: se puede cambiar cualquier campo de un miembro
   excepto su `id` y el `id` de su jefe. Tras cada cambio se vuelve a
   verificar la jefatura.
5. **Guardar cambios**: escribe el árbol de vuelta al CSV.

## Datos de prueba

`bin/familia.csv` incluye 15 miembros inspirados en *El Padrino*: Vito (jefe,
68 años), un hijo muerto (Santino), miembros en la cárcel (Fredo, Carlo) y
varias ramas para probar todas las reglas de sucesión. Por ejemplo, subir la
edad de Vito a 71 o marcarlo como muerto dispara la sucesión automática hacia
Michael.
