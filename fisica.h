#ifndef FISICA_H
#define FISICA_H

// Recibe 2 extremos de una recta (x0, y0) y (x1, y1) y un punto (xp, yp)
// y devuelve en (x, y) la distancia entre la recta y el punto
void punto_mas_cercano(float x0, float y0, float x1, float y1, float xp, float yp, float *x, float *y);

// Recibe una normal (norm_x, norm_y), un punto (cx, cy) y una velocidad (vx, vy)
// y computa el rebote contra un objeto con esa normal
// modificando tanto el punto como la velicidad.
void reflejar(float norm_x, float norm_y, float *cx, float *cy, float *vx, float *vy);

// Dada una velocidad actual vi, una aceleracion a y un paso temporal dt
// devuelve la velocidad del proximo instante
double computar_velocidad(double vi, double a, double dt);

// Dada una posision actual, una velocidad actual y un paso temporal dt
// devuelve la posision del proximo instante
double computar_posicion(double pi, double vi, double dt);


// Recibe 2 vectores (x1, y1) y (x2, y2)
// y devuelve el producto interno entre ellos
float producto_interno(float x1, float y1, float x2, float y2);

// Recibe 2 puntos (x0,y0) y (x1, y1)
// y devuelve la distancia entre los 2 puntos
double distancia(float x0,float y0,float x1,float y1);



#endif