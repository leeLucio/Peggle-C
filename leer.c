#include <stdlib.h>
#include "leer.h"
#include "config.h"

#define MASK_COLOR 0xC0
#define MASK_MOVIMIENTO 0x30
#define MASK_GEOMETRIA 0x0F

#define SHIFT_COLOR 6
#define SHIFT_MOVIMIENTO 4


static bool leer_movimiento_inmovil(FILE *f, int16_t parametros[], size_t *n_parametros);
static bool leer_movimiento_circular(FILE *f, int16_t parametros[], size_t *n_parametros);
static bool leer_movimiento_horizontal(FILE *f, int16_t parametros[], size_t *n_parametros);

typedef bool (*tipo_mov_t)(FILE *, int16_t *, size_t *);

tipo_mov_t tipo_mov[] = {
	[INMOVIL] = leer_movimiento_inmovil,
	[CIRCULAR] = leer_movimiento_circular,
	[HORIZONTAL] = leer_movimiento_horizontal
};

static poligono_t *leer_geometria_circulo(FILE *f);
static poligono_t *leer_geometria_rectangulo(FILE *f);
static poligono_t *leer_geometria_poligono(FILE *f);


typedef poligono_t* (*tipo_geometria_t)(FILE *);

tipo_geometria_t tipo_geometria[] = {
	[CIRCULO] = leer_geometria_circulo,
	[RECTANGULO] = leer_geometria_rectangulo,
	[POLIGONO] = leer_geometria_poligono
};


bool leer_encabezado(FILE *f, color_t *color, movimiento_t *movimiento, geometria_t *geometria){
	uint8_t aux;
	if(fread(&aux, 1, 1, f) != 1)
		return false;
	
	if((*color = (aux & MASK_COLOR) >> SHIFT_COLOR) > 3)
		return false;

	if((*movimiento = (aux & MASK_MOVIMIENTO) >> SHIFT_MOVIMIENTO) > 2)
		return false;

	if((*geometria = aux & MASK_GEOMETRIA) > 2)
		return false;


	return true;
}

static bool leer_movimiento_inmovil(FILE *f, int16_t parametros[], size_t *n_parametros){
	*n_parametros = 0;
	return true;
}

static bool leer_movimiento_circular(FILE *f, int16_t parametros[], size_t *n_parametros){
	return (*n_parametros = fread(parametros, sizeof(int16_t), 3, f)) == 3; 
}

static bool leer_movimiento_horizontal(FILE *f, int16_t parametros[], size_t *n_parametros){
	return (*n_parametros = fread(parametros, sizeof(int16_t), 3, f)) == 3;
}


bool leer_movimiento(FILE *f, movimiento_t movimiento, int16_t parametros[], size_t *n_parametros){
	return tipo_mov[movimiento](f, parametros, n_parametros);
}


static poligono_t *leer_geometria_circulo(FILE *f){
	int16_t aux[3]; // [x][y][radio]
	
	if(fread(aux, sizeof(int16_t), 3, f) != 3)
		return NULL;

	poligono_t *poligono = poligono_crear_circular(aux[0], aux[1], aux[2]);
	if (poligono == NULL)
		return NULL;

	return poligono;
}

static poligono_t *leer_geometria_rectangulo(FILE *f){
	int16_t aux[5];
	//"x", "y", "ancho", "alto", "angulo"

	if(fread(aux, sizeof(int16_t), 5, f) != 5)
		return NULL;

	float cx = aux[0];
	float cy = aux[1];

	float vertices[4][2];

	vertices[0][0] = aux[2] / 2;
	vertices[0][1] = aux[3] / 2;

	for(size_t i = 1; i < 4; i++){
		if(i % 2){
			vertices[i][0] = -vertices[i-1][0];
			vertices[i][1] = vertices[i-1][1];
		}
		else{
			vertices[i][0] = vertices[i-1][0];
			vertices[i][1] = -vertices[i-1][1];				
		}
	}

	poligono_t *p = poligono_crear(vertices, 4);
	if(p == NULL)
		return NULL;

	double radianes = aux[4] * (PI / 180);
	poligono_rotar(p, radianes);
	poligono_trasladar(p, cx, cy);

	return p;
}

static poligono_t *leer_geometria_poligono(FILE *f){
	int16_t n_puntos;
	fread(&n_puntos, sizeof(int16_t), 1, f);

	int16_t (*aux)[2] = malloc(sizeof(int16_t[2]) * n_puntos);
	fread(aux, sizeof(int16_t), n_puntos * 2, f);

	float (*flotantes)[2] = malloc(sizeof(float[2]) * n_puntos);

	for(size_t i = 0; i < n_puntos; i++){
		flotantes[i][0] = aux[i][0];
		flotantes[i][1] = aux[i][1];

	}
	
	
	poligono_t *poligono = poligono_crear(flotantes, n_puntos);
	if(poligono == NULL){
		free(flotantes);
		return NULL;
	}
	free(aux);
	free(flotantes);
	return poligono;
}

poligono_t *leer_geometria(FILE*f, geometria_t geometria){
	return tipo_geometria[geometria](f);	
}

