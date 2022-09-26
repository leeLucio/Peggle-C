#ifndef POLIGONO_H
#define POLIGONO_H

#include <stddef.h>
#include <stdbool.h>
#include <SDL2/SDL.h>


// Defincion del tipo de dato
struct poligono;
typedef struct poligono poligono_t;

// Crea un poligono a partir de los n vertices recibidos
// Post: Se devuelve un poligono nuevo en memoria.
// Devuelve NULL en caso de fallar
poligono_t *poligono_crear(float vertices[][2], size_t n);

//Crea un poligono circular de radio radio con centro en la posicion (cx, cy)
// Post: Se devuelve un poligono nuevo en memoria
// Devuelve NULL en caso de fallar
poligono_t *poligono_crear_circular(float cx, float cy, float radio);

// Copia el poligono recibido en memoria nueva 
// Pre: El poligono existe
// Post: Devuelve el poligono nuevo en memoria.
// Devuelve NULL en caso de fallar
poligono_t *poligono_clonar(const poligono_t *poligono);

// Destruye el poligono recibido
void poligono_destruir(poligono_t *poligono);

// Devuelve la cantidad de vertices del poligono recibido
// Pre: El poligono existe
size_t poligono_cantidad_vertices(const poligono_t *poligono);

// Devuelve en (x,y) el vertice de la posicion pos que contiene el poligono recibido
// Devuelve false por el nombre en caso de no encontrarlo
// Pre: El polgiono existe
bool poligono_obtener_vertice(const poligono_t *poligono, size_t pos, float *x, float *y);

// Agrega el vertice (x, y) al final del poligono
// Devuelve false por el nombre en caso de fallar la operacion 
bool poligono_agregar_vertice(poligono_t *poligono, float x, float y);

// Modifica cada uno de los componentes del poligono
// mediante la adicioin del par (dx, dy) 
void poligono_trasladar(poligono_t *poligono, float dx, float dy);

// Recibe un poligono y rota cada uno de sus puntos en rad radianes
// respecto al origen de coordenadas
void poligono_rotar(poligono_t *poligono, double rad);

// Recibe un poligono y rota cada uno de sus puntos en ang radianes
// respecto a un punto (cx, cy)
void poligono_rotar_centrado(poligono_t *poligono, float cx, float cy, double ang);

// Recibe un polígono p, un punto (cx, cy) y devuelve la distancia del punto al polígono por el nombre
// y las componentes (norm_x, norm_y) de la normal al polígono en el punto más cercano.
double poligono_distancia(const poligono_t *p, float cx, float cy, float *norm_x, float *norm_y);

// Recibe un poligono y lo dibuja sobre el renderer
void poligono_dibujar(poligono_t *poligono, SDL_Renderer *renderer);

#endif // POLIGONO_H