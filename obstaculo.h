#ifndef OBSTACULO_H
#define OBSTACULO_H

#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdbool.h>

#include "tipos.h"

// Deficion del tipo de dato
struct obstaulo;
typedef struct obstaculo obstaculo_t;

// Dado un archivo f, lee de el los bytes necesarios para crear un obstaculo
// Devuelve un obstaculo nuevo creado en memoria.
// Devuelve NULL en caso de fallar.
obstaculo_t *obstaculo_crear_de_archivo(FILE *f);

// Copia el obstaculo recibido en memoria nueva
// Devuelve NULL en caso de fallar
obstaculo_t *obstaculo_clonar(obstaculo_t *obs);

// Crea un obstaculo a partir de los n vertices recibidos.
// El obstaculo devuelto es del color color
// y el movimiento es un movimiento que se mueve segun mov_parametros
obstaculo_t *obstaculo_crear(float vertices[][2], size_t n, color_t color, movimiento_t movimiento, float mov_parametros[]);

// Devuelve el color del obstaculo recibido
// Pre: El obstaculo existe
color_t obstaculo_obtener_color(obstaculo_t *obstaculo);

// Recibe un obstaculo y un punto (cx, cy)
// Devuelve por el nombre la distancia del punto al obstaculo
// y las componenetes (norm_x, norm_y) de la normal al obstaculo en el punto mas cercano.
double obstaculo_distancia(obstaculo_t *obstaculo, float cx, float cy, float *norm_x, float *norm_y);

// Actualiza la posicion del obstaculo segun su movimiento
void obstaculo_computar_posicion(obstaculo_t *obstaculo);

// Dado el obstaculo recibido marca al obstaculo y lo vuelve amarillo
void obstaculo_golpear(obstaculo_t *obstaculo);

// Verifica si el obstaculo ya fue golpeado.
bool obstaculo_fue_golpeado(obstaculo_t *obstaculo);

// Destruye el obstaculo recibido
void obstaculo_destruir(obstaculo_t *obstaculo);

// Dibuja el obstaculo recibido sobre el renderer
void obstaculo_dibujar(SDL_Renderer *renderer, obstaculo_t *obstaculo);

#endif // OBSTACULO_H