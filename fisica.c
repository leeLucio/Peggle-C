#include <math.h>

#include "fisica.h"

double computar_velocidad(double vi, double a, double dt){
	 return dt * a + vi; // 
}

double computar_posicion(double pi, double vi, double dt){
	return dt * vi + pi;
}

double distancia(float x0,float y0,float x1,float y1){
	float vect_x = x0 - x1;
	float vect_y = y0 - y1;

	return sqrt(vect_x * vect_x + vect_y * vect_y);
}

void punto_mas_cercano(float x0, float y0, float x1, float y1, float xp, float yp, float *x, float *y){
	float ax = xp - x0;
    float ay = yp - y0;
    float bx = x1 - x0;
    float by = y1 - y0;

    float alfa = producto_interno(ax, ay, bx, by) / producto_interno(bx, by, bx, by);

    if(alfa <= 0) {
        *x = x0;
        *y = y0;
    }
    else if(alfa >= 1) {
        *x = x1;
        *y = y1;
    }
    else {
        *x = alfa * bx + x0;
        *y = alfa * by + y0;
    }

}

float producto_interno(float x1, float y1, float x2, float y2){
	return x1 * x2 + y1 * y2;
}


void reflejar(float norm_x, float norm_y, float *cx, float *cy, float *vx, float *vy){
	float proy = producto_interno(norm_x, norm_y, *vx, *vy);

    if(proy >= 0)
        return;

    *vx -= 2 * norm_x * proy;
    *vy -= 2 * norm_y * proy;

    // Además empujamos a la bola hacia afuera para que no se pegue
    *cx += norm_x * 0.1;
    *cy += norm_y * 0.1;
}


