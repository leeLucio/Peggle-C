#ifndef LEER_H
#define LEER_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "tipos.h"
#include "poligono.h"

// Dado un archivo f, lee de él 1 byte
// Devuelve por la interfaz el color, movimiento y geometria
// Devuelve true si todo es correcto
bool leer_encabezado(FILE *f, color_t *color, movimiento_t *movimiento, geometria_t *geometria);


// Dado el archivo f, lee de el el movimiento correspondiente al movimiento dado
// Almacena en parametros los parametros que representan el movimiento
// Devuelve true por el nombre en caso de estar todo correcto
// Devuelve por la interfaz n_parametros con la cantidad de parametros leidos
bool leer_movimiento(FILE *f, movimiento_t movimiento, int16_t parametros[], size_t *n_parametros);


/*
Dado el archivo f, lee de el la geometria del tipo dado y 
devuelve un poligono nuevo creado en memoria con los datos leidos.
Devuelve NULL en caso de fallar
*/
poligono_t *leer_geometria(FILE*f, geometria_t geometria);



#endif // LEER_H