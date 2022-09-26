#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <SDL2/SDL.h>

#include "poligono.h"
#include "tipos.h"
#include "config.h"
#include "fisica.h"

struct poligono{
    float (*vertices)[2];
    size_t n;
};



poligono_t *poligono_crear(float vertices[][2], size_t n){
   
    poligono_t *poligono = malloc(sizeof(poligono_t));
    
    if (poligono == NULL)
        return NULL;

    poligono->vertices = malloc( 2* n * sizeof(float));

    if(poligono->vertices == NULL){
        free(poligono);
        return NULL;
    }
    
    for (size_t i = 0; i < n; i++){
        poligono->vertices[i][0] = vertices[i][0];
        poligono->vertices[i][1] = vertices[i][1];
    }

    poligono->n = n;

    return poligono;
}

poligono_t *poligono_crear_circular(float cx, float cy, float radio){
	poligono_t *poligono = malloc(sizeof(poligono_t));
	if ( poligono == NULL)
		return NULL;

	poligono->n = CIRCULO_RES;

	poligono->vertices = malloc(sizeof(float[2]) * poligono->n);
	if(poligono->vertices == NULL){
		free(poligono);
		return NULL;
	}
	double radianes = 0;
	
	for(size_t i = 0; i < poligono->n; i++){
		poligono->vertices[i][0] = radio * cos(radianes);
		poligono->vertices[i][1] = radio * sin(radianes);

		radianes += 2 * PI / CIRCULO_RES;
	}

	poligono_trasladar(poligono, cx, cy);
	
	return poligono;
}


void poligono_destruir(poligono_t *poligono){
    free(poligono->vertices);
    free(poligono);
}


size_t poligono_cantidad_vertices(const poligono_t *poligono){
    return poligono->n;
}


bool poligono_obtener_vertice(const poligono_t *poligono, size_t pos, float *x, float *y){
    if (pos >= poligono->n)
        return false;

    *x = poligono->vertices[pos][0];
    *y = poligono->vertices[pos][1];

    return true;    
}


poligono_t *poligono_clonar(const poligono_t *poligono){
    poligono_t *clon = poligono_crear(poligono->vertices, poligono->n);
    
    if (clon == NULL)
        return NULL;

    return clon;
}


bool poligono_agregar_vertice(poligono_t *poligono, float x, float y){
    float (*aux)[2] = realloc(poligono->vertices, 2 * (poligono->n + 1) * sizeof(float));
    
    if(aux == NULL)
        return false;

    poligono->vertices = aux;

    poligono->vertices[poligono->n][0] = x;
    poligono->vertices[poligono->n][1] = y;

    poligono->n += 1;
    return true;
}

void poligono_trasladar(poligono_t *poligono, float dx, float dy){
	for (int i = 0; i < poligono->n; i++){
		poligono->vertices[i][0] += dx;
		poligono->vertices[i][1] += dy;
	}
}

void poligono_rotar(poligono_t *poligono, double rad){
	for (int i = 0; i < poligono->n; i++){
		
		float x_prima = poligono->vertices[i][0] * cos(rad) - poligono->vertices[i][1] * sin(rad);
		float y_prima = poligono->vertices[i][0] * sin(rad) + poligono->vertices[i][1] * cos(rad);
		
		poligono->vertices[i][0]=x_prima;
		poligono->vertices[i][1]=y_prima;
	}
}

void poligono_rotar_centrado(poligono_t *poligono, float cx, float cy, double ang){
    poligono_trasladar(poligono, -cx, -cy);
    poligono_rotar(poligono, ang);

    poligono_trasladar(poligono, cx, cy);
}


double poligono_distancia(const poligono_t *p, float xp, float yp, float *nor_x, float *nor_y) {
    double d = 1 / 0.0;
    size_t idx = 0;

    for(size_t i = 0; i < p->n; i++) {
        float xi, yi;
        punto_mas_cercano(p->vertices[i][0], p->vertices[i][1], p->vertices[(i + 1) % p->n][0], p->vertices[(i + 1) % p->n][1], xp, yp, &xi, &yi);
        double di = distancia(xp, yp, xi, yi);

        if(di < d) {
            d = di;
            idx = i;
        }
    }

    float nx = p->vertices[(idx + 1) % p->n][1] - p->vertices[idx][1];
    float ny = p->vertices[idx][0] - p->vertices[(idx + 1) % p->n][0];
    float dn = distancia(nx, ny, 0, 0);

    nx /= dn;
    ny /= dn;

    *nor_x = nx;
    *nor_y = ny;

    return d;
}

void poligono_dibujar(poligono_t *poligono, SDL_Renderer *renderer){
	size_t n_vertices = poligono->n;
	
	for(size_t i = 0; i < n_vertices-1; i++){
		SDL_RenderDrawLine(renderer, poligono->vertices[i][0], poligono->vertices[i][1], poligono->vertices[i+1][0], poligono->vertices[i+1][1]);
	}

	SDL_RenderDrawLine(renderer, poligono->vertices[n_vertices-1][0], poligono->vertices[n_vertices-1][1], poligono->vertices[0][0], poligono->vertices[0][1]);	
}