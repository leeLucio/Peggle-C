#include <stdint.h>
#include <SDL2/SDL.h>

#include "obstaculo.h"
#include "leer.h"
#include "fisica.h"
#include "config.h"
#include "poligono.h"

struct obstaculo{
	poligono_t *poligono;
	color_t color;
	movimiento_t movimiento;
	float mov_parametros[3];
	bool golpeado;
};

void obstaculo_mover_circular(obstaculo_t *obstaculo);
void obstaculo_mover_horizontal(obstaculo_t *obstaculo);

typedef void (*obstaculo_mover_t)(obstaculo_t *);

obstaculo_mover_t funcion_mover[]={
	[CIRCULAR] = obstaculo_mover_circular, 
	[HORIZONTAL] = obstaculo_mover_horizontal
};


static const uint8_t color_arr[][3]={
	[AZUL] = {0x00, 0x00, 0xFF},
	[NARANJA] = {0xFF, 0xA0, 0x00},
	[VERDE] = {0x00, 0xFF, 0x00},
	[GRIS] = {0x7D, 0x7D, 0x7D},
	[AMARILLO] = {0xFF, 0XFF, 0X00}
};


obstaculo_t *obstaculo_crear_de_archivo(FILE *f){
	color_t color;
	movimiento_t movimiento;
	geometria_t geometria;

	if(! leer_encabezado(f, &color, &movimiento, &geometria))
		return NULL;

	obstaculo_t *obs = malloc(sizeof(obstaculo_t));
	if (obs == NULL)
		return NULL;

	obs->color = color;
	obs->movimiento = movimiento;

	obs->golpeado = false;

	
	size_t n_parametros;
	int16_t aux[3];
	if (! leer_movimiento(f, movimiento, aux, &n_parametros)){
		free(obs);
		return NULL;
	}

	for (size_t i = 0; i < 3; i++){
		obs->mov_parametros[i] = aux[i];
	}
	

	obs->poligono = leer_geometria(f, geometria);
	if(obs->poligono == NULL){
		free(obs);
		return NULL;
	}

	return obs;
}

obstaculo_t *obstaculo_clonar(obstaculo_t *obs){
	obstaculo_t *clon = malloc(sizeof(obstaculo_t));
	if(clon == NULL)
		return NULL;

	clon->poligono = poligono_clonar(obs->poligono);
	if(clon->poligono == NULL){
		free(clon);
		return NULL;
	}

	clon->color = obs->color;
	clon->golpeado = obs->golpeado;
	clon->movimiento = obs->movimiento;
	
	memcpy(clon->mov_parametros, obs->mov_parametros, sizeof(float) * 3);

	return clon;
}

obstaculo_t *obstaculo_crear(float vertices[][2], size_t n, color_t color, movimiento_t movimiento, float mov_parametros[]){
	obstaculo_t *obstaculo = malloc(sizeof(obstaculo_t));
	if(obstaculo == NULL)
		return NULL;

	poligono_t *poligono = poligono_crear(vertices, n);
	if(poligono == NULL){
		free(obstaculo);
		return NULL;
	}

	obstaculo->poligono = poligono;
	
	obstaculo->color = color;
	obstaculo->golpeado = false;
	obstaculo->movimiento = movimiento;

	memcpy(obstaculo->mov_parametros, mov_parametros, sizeof(float) * 3);

	return obstaculo;
}

color_t obstaculo_obtener_color(obstaculo_t *obstaculo){
	return obstaculo->color;
}

void obstaculo_computar_posicion(obstaculo_t *obstaculo){
	if(obstaculo->movimiento == INMOVIL)
		return;

	funcion_mover[obstaculo->movimiento](obstaculo);
}


void obstaculo_mover_circular(obstaculo_t *obstaculo){
	poligono_rotar_centrado(obstaculo->poligono, obstaculo->mov_parametros[0], obstaculo->mov_parametros[1], obstaculo->mov_parametros[2] / JUEGO_FPS);
}


void obstaculo_mover_horizontal(obstaculo_t *obstaculo){
	float x1 = obstaculo->mov_parametros[0];
	float xi = obstaculo->mov_parametros[1];

	if(xi > x1 || xi < 0)
		obstaculo->mov_parametros[2] *= -1;

	poligono_trasladar(obstaculo->poligono, obstaculo->mov_parametros[2] / JUEGO_FPS, 0);
	obstaculo->mov_parametros[1] += obstaculo->mov_parametros[2] / JUEGO_FPS;
}



double obstaculo_distancia(obstaculo_t *obstaculo, float cx, float cy, float *norm_x, float *norm_y){
	return poligono_distancia(obstaculo->poligono, cx, cy, norm_x, norm_y);
}


void obstaculo_golpear(obstaculo_t *obstaculo){
	if(obstaculo->color == GRIS)
		return;

	obstaculo->golpeado = true;
	obstaculo->color = AMARILLO;
}


bool obstaculo_fue_golpeado(obstaculo_t *obstaculo){
	return obstaculo->golpeado;
}


void obstaculo_destruir(obstaculo_t *obstaculo){
	poligono_destruir(obstaculo->poligono);
	free(obstaculo);
}


void obstaculo_dibujar(SDL_Renderer *renderer, obstaculo_t *obstaculo){
	color_t color = obstaculo->color;
	SDL_SetRenderDrawColor(renderer, color_arr[color][0], color_arr[color][1], color_arr[color][2], 0x00);

	poligono_dibujar(obstaculo->poligono, renderer);
}
