#include <stdbool.h>
#include <SDL2/SDL.h>

#include "config.h"
#include "tipos.h"
#include "poligono.h"
#include "fisica.h"
#include "leer.h"
#include "lista.h"
#include "obstaculo.h"

#define DT (1.0 / JUEGO_FPS)

// Para hacer un arreglo de componentes para la trayectoria
typedef struct{
	double x;
	double y;
	double vx;
	double vy;
}componentes_t;

// Puntaje de cada color
const unsigned short valor_colores[] = {
	[NARANJA] = 100,
	[AZUL] = 10,
	[VERDE] = 0,
	[GRIS] = 0
};

// Componentes RGB de cada color
const uint8_t color_arr[][3]={
	[AZUL] = {0x00, 0x00, 0xFF},
	[NARANJA] = {0xFF, 0xA0, 0x00},
	[VERDE] = {0x00, 0xFF, 0x00},
	[GRIS] = {0x7D, 0x7D, 0x7D},
	[AMARILLO] = {0xFF, 0XFF, 0X00},
	[BLANCO] = {0xFF, 0xFF, 0xFF}
};


#ifdef TTF
#include <SDL2/SDL_ttf.h>

void escribir_texto(SDL_Renderer *renderer, TTF_Font *font, const char *s, int x, int y, color_t color_){
	SDL_Color color = {color_arr[color_][0], color_arr[color_][1], color_arr[color_][2]};  // Estaría bueno si la función recibe un enumerativo con el color, ¿no?
    SDL_Surface *surface = TTF_RenderText_Solid(font, s, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = surface->w;
    rect.h = surface->h;

    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}
#endif




// Wrapper para lista_destruir()
void destruir_obstaculo(void *dato){
	obstaculo_destruir(dato);
}

// Wrapper para lista_iterar()
bool dibujar_obstaculo(void *dato, void *extra){
	if(dato == NULL)
		return false;

	obstaculo_dibujar(extra, dato);
	return true;
}


int main(int argc, char *argv[]) {
    if(argc != 2) {
		fprintf(stderr, "Uso: %s <archivo>\n", argv[0]);
		return 1;
	}

	FILE *f = fopen(argv[1], "rb");
	if(f == NULL) {
		fprintf(stderr, "No pudo abrirse \"%s\"\n", argv[1]);
		return 1;
	}

	SDL_Init(SDL_INIT_VIDEO);

#ifdef TTF
    TTF_Init();
    TTF_Font* font = TTF_OpenFont("FreeSansBold.ttf", 24);
    TTF_Font* font_chiquito = TTF_OpenFont("FreeSansBold.ttf", 14);
#endif

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;

    SDL_CreateWindowAndRenderer(VENTANA_ANCHO, VENTANA_ALTO, 0, &window, &renderer);
    SDL_SetWindowTitle(window, "Peggle");

    int dormir = 0;

    // BEGIN código del alumno
	
	size_t nivel = 0;
	size_t puntaje_total = 0;
	char puntaje_total_text[100];
	sprintf(puntaje_total_text, "Puntaje total: %ld", puntaje_total);

	bool fallo_algo = false;
	bool reiniciar_nivel = false; // true: Hay que reiniciar el nivel

	while(1){
		// Creo una lista original al principio del nivel que no se va a modificar hasta que se termine el nivel
		lista_t *lista_original;
		if(! reiniciar_nivel){
			nivel++;
			
			int16_t obstaculos;
			if(! fread(&obstaculos, sizeof(int16_t), 1, f)){
				break;
			}

			lista_original = lista_crear();
			if(lista_original == NULL){
				fprintf(stderr, "No se pudo crear la lista\n");
				fallo_algo = true;
				break;
			}
				

			for(size_t i = 0; i < obstaculos; i++) {
				obstaculo_t *obs = obstaculo_crear_de_archivo(f);
				if(obs == NULL){
					fallo_algo = true;
					lista_destruir(lista_original, destruir_obstaculo);
					break;
				}
				
				if(! lista_insertar_primero(lista_original, obs)){
					fallo_algo = true;
					lista_destruir(lista_original, destruir_obstaculo);
					break;
				}

			}

			if(fallo_algo) break;
		}

		// Creo un clon de la lista que va a ser la que se use en cada nivel
		// y que se va a ir modificando dentro de la iteracion
		lista_t *lista = lista_crear();
		if(lista == NULL){
			fallo_algo = true;
			lista_destruir(lista_original, destruir_obstaculo);
			break;
		}

		lista_iter_t *lista_iter_clonar = lista_iter_crear(lista_original);
		if(lista_iter_clonar == NULL){
			lista_destruir(lista_original, destruir_obstaculo);
			lista_destruir(lista, destruir_obstaculo);

			fallo_algo = true;
			break;
		}

		// Copio cada obstaculo y los agrego a la nueva lista
		while(! lista_iter_al_final(lista_iter_clonar)){
			obstaculo_t *obs = lista_iter_ver_actual(lista_iter_clonar);

			obstaculo_t *clon = obstaculo_clonar(obs);
			if(clon == NULL){
				lista_destruir(lista_original, destruir_obstaculo);
				lista_destruir(lista, destruir_obstaculo);

				fallo_algo = true;
				break;
			}

			if(! lista_insertar_primero(lista, clon)){
				lista_destruir(lista_original, destruir_obstaculo);
				lista_destruir(lista, destruir_obstaculo);

				fallo_algo = true;
				break;
			}

			lista_iter_avanzar(lista_iter_clonar);
		}
		lista_iter_destruir(lista_iter_clonar);
		
		if(fallo_algo) break;

		float canon_angulo = 0; // Ángulo del cañón
		bool cayendo = false;   // ¿Hay bola disparada?

		float cx, cy;   // Centro de la bola
		float vx, vy;   // Velocidad de la bola

		unsigned int vidas = 20; // Se comienza el nivel con 10 vidas
		char texto_vidas[3];

		reiniciar_nivel = false; // Al comienzo de nivel no hay necesidad de reiniciar

		unsigned int tiempo_quieto = 0; // La cantidad que la bola se mantiene "quieta"

		char texto_nivel[20];
		sprintf(texto_nivel, "Nivel %zd",nivel);

		size_t puntaje_nivel = 0;
		char texto_puntaje_nivel[100];
		sprintf(texto_puntaje_nivel, "Puntaje nivel: %ld", puntaje_nivel);
		
		unsigned short multiplicador = 1;		// Multiplicador de puntos
		unsigned short naranjas_golpeados = 0; 	// Cantidad de naranjas golpeados 

		char texto_golpeados[3];		// En esta cadena se almacena la cantidad de naranjas golpeados  

		char texto_multiplicador[30];
		sprintf(texto_multiplicador, "x%d", multiplicador);

		bool eliminar = false;		// true: Hay algun obstaculo que eliminar, false: No hay nada que eliminar


		// Creo el recuperador de pelotas
		float vertices[][2]= {
			{80 + (MAX_X - MIN_X)/2 + MIN_X, MAX_Y},
			{-80 +( MAX_X - MIN_X)/2 + MIN_X, MAX_Y},
			{-80 +( MAX_X - MIN_X)/2 + MIN_X, MAX_Y-10},
			{80 + (MAX_X - MIN_X)/2 + MIN_X, MAX_Y-10}
		};

		float parametros[]={MAX_X - MIN_X - 160, (MAX_X - MIN_X - 160)/2, 200};

		obstaculo_t *recuperador = obstaculo_crear(vertices, 4, VERDE, HORIZONTAL, parametros);
		if(recuperador == NULL){
			lista_destruir(lista_original, destruir_obstaculo);
			lista_destruir(lista, destruir_obstaculo);

			fallo_algo = true;
			break;
		}
		// END código del alumno

		unsigned int ticks = SDL_GetTicks();

		while(1) {
			if(SDL_PollEvent(&event)) {
				if (event.type == SDL_QUIT)
					break;

				// BEGIN código del alumno
				if(event.type == SDL_MOUSEBUTTONDOWN) {
					if(! cayendo){
						vidas--; 		// Resto una vida cuando disparo
						cayendo = true;
					}
				}
				else if (event.type == SDL_MOUSEMOTION) {
					canon_angulo = atan2(event.motion.x - CANON_X, event.motion.y - CANON_Y);
					if(canon_angulo > CANON_MAX)
						canon_angulo = CANON_MAX;
					if(canon_angulo < -CANON_MAX)
						canon_angulo = -CANON_MAX;
				}
				// END código del alumno

				continue;
			}
			SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
			SDL_RenderClear(renderer);
			SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);


			// BEGIN código del alumno
			sprintf(texto_multiplicador, "x%d", multiplicador); // Actualizo la cadena de multiplicador
			sprintf(texto_golpeados, "%d", naranjas_golpeados); // Actualizo la cadena de la cant de golpeados
			sprintf(texto_vidas, "%d", vidas); 				// Actualizo la cadena de las vidas	


			if(! cayendo){
				sprintf(texto_puntaje_nivel, "Puntaje nivel: %ld", puntaje_nivel);
			}


	#ifdef TTF	
			escribir_texto(renderer, font, texto_puntaje_nivel, 80, 20, BLANCO);
			escribir_texto(renderer, font, texto_nivel, 350, 20, BLANCO);
			escribir_texto(renderer, font, puntaje_total_text, 500, 20, BLANCO);

			escribir_texto(renderer, font_chiquito, "Naranjas", 725, 50, NARANJA);
			escribir_texto(renderer, font_chiquito, "golpeados", 722, 70, NARANJA);
			escribir_texto(renderer, font, texto_golpeados, 750, 90, NARANJA);

			escribir_texto(renderer, font_chiquito, "Puntos", 735, 500, NARANJA);
			escribir_texto(renderer, font, texto_multiplicador, 740, 515, NARANJA);
			
			escribir_texto(renderer, font_chiquito, "Vidas", 20, 50, VERDE);
			escribir_texto(renderer, font, texto_vidas, 20, 70, VERDE);
	#endif

			
			if(cayendo) {
				// Si la bola está cayendo actualizamos su posición
				vy = computar_velocidad(vy, G, DT);
				vx *= ROZAMIENTO;
				vy *= ROZAMIENTO;
				cx = computar_posicion(cx, vx, DT);
				cy = computar_posicion(cy, vy, DT);
			}
			else {
				// Si la bola no se disparó establecemos condiciones iniciales
				cx = CANON_X + CANON_LARGO * sin(canon_angulo);
				cy = CANON_Y + CANON_LARGO * cos(canon_angulo);
				vx = BOLA_VI * sin(canon_angulo);
				vy = BOLA_VI * cos(canon_angulo);
			}


			// Contamos el tiempo que la bola esta "quieta"
			if(cayendo){
				if((vx < 20 && vy < 20) && (vx > -20 && vy > -20)){
					tiempo_quieto++;
				}
				else tiempo_quieto = 0;
			}

			// Actualizo la posicion del recuperador
			obstaculo_computar_posicion(recuperador);

			// Actualizo el multiplicador
			switch(naranjas_golpeados){
				case 10 : multiplicador = 2;
					break;

				case 15 : multiplicador = 3;
					break;
				
				case 19 : multiplicador = 5;
					break;
				
				case 21 : multiplicador = 10;
					break;

				default :
					break;
			}


			//Si la lista esta vacia o si ya no quedan obstaculos naranjas termina el nivel
			if(! cayendo){
				// Si la lista esta vacia, termina el nivel
				if(lista_esta_vacia(lista))
					break;

				lista_iter_t *lista_iter_naranja = lista_iter_crear(lista);
				if(lista_iter_naranja == NULL){
					fallo_algo = true;
					lista_iter_destruir(lista_iter_naranja);
					break;
				}

				
				while(! lista_iter_al_final(lista_iter_naranja)){
					obstaculo_t *actual = lista_iter_ver_actual(lista_iter_naranja);
					if(actual == NULL)
						break;

					color_t color_actual = obstaculo_obtener_color(actual);
					if(color_actual == NARANJA)
						break;
					
					lista_iter_avanzar(lista_iter_naranja);
				}
				
				// Si la lista llegó al final, no hay naranjas => Termina el nivel
				if(lista_iter_al_final(lista_iter_naranja)){
					lista_iter_destruir(lista_iter_naranja);
					break;
				}
				lista_iter_destruir(lista_iter_naranja);
			}


			// Si no me quedan mas vidas y aun quedan naranjas, se reinicia el nivel
			if(vidas == 0 && !cayendo){
				reiniciar_nivel = true;
				break;
			}


			lista_iter_t *lista_iter = lista_iter_crear(lista);
			if(lista_iter == NULL){
				fallo_algo = true;
				break;
			}

			// Actualizo los obstaculos y aplico rebote
			while(! lista_iter_al_final(lista_iter)){
				obstaculo_t *actual = lista_iter_ver_actual(lista_iter);
				if(actual == NULL)
					break;

				obstaculo_computar_posicion(actual);
				
				// Rebote contra los obstaculos
				float norm_x, norm_y;
				if(obstaculo_distancia(actual, cx, cy, &norm_x, &norm_y) < BOLA_RADIO){
					reflejar(norm_x, norm_y, &cx, &cy, &vx, &vy);

					vx *= PLASTICIDAD;
					vy *= PLASTICIDAD;

					// Golpeo los obstaculos y sumo los puntos
					if(! obstaculo_fue_golpeado(actual)){
						color_t color = obstaculo_obtener_color(actual);
						puntaje_nivel += valor_colores[color] * multiplicador;
						obstaculo_golpear(actual);

						if(color == NARANJA)
							naranjas_golpeados++;
						
						if(color == VERDE)
							vidas++;
						
						if(color != GRIS)
							eliminar = true;
					}

				}



				// Elimino los obstaculos si la bola se salio de la pantalla o si la pelota esta quieta
				if(!cayendo || tiempo_quieto >= JUEGO_FPS * 3){
					if(obstaculo_fue_golpeado(actual)){
						obstaculo_t *obs = lista_iter_borrar(lista_iter);
						if(obs == NULL)
							break;

						obstaculo_destruir(actual);
					}
				}

				if(! lista_iter_avanzar(lista_iter))
					break;
			}
		
			lista_iter_destruir(lista_iter);
			

			poligono_t *bola = poligono_crear_circular(cx, cy, BOLA_RADIO);
			if(bola == NULL){
				fallo_algo = true;
				break;
			}

			// Rebote contra las paredes:
			if(cx < MIN_X + BOLA_RADIO || cx > MAX_X - BOLA_RADIO) vx = - vx;
			if(cy < MIN_Y + BOLA_RADIO) vy = -vy;

			// Se salió de la pantalla:
			if(cy > MAX_Y - BOLA_RADIO){
				// Si no habia ningun obstaculo que eliminar, recupero la vida
				if(! eliminar)
					vidas++;
		
				cayendo = false;
				eliminar = false;
			}

			float norm_x, norm_y;
			// Si la bola cae en el recuperador, sumo una vida
			if(obstaculo_distancia(recuperador, cx, cy, &norm_x, &norm_y) < BOLA_RADIO){
				vidas++;
				cayendo = false;
			}


			// Dibujamos el cañón:
			SDL_RenderDrawLine(renderer, CANON_X, CANON_Y, CANON_X + sin(canon_angulo) * CANON_LARGO, CANON_Y + cos(canon_angulo) * CANON_LARGO);

			// Dibujamos la bola:
			poligono_dibujar(bola, renderer);

			// Dibujamos las paredes:
			SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0x00);
			SDL_RenderDrawLine(renderer, MIN_X, MIN_Y, MAX_X, MIN_Y);
			SDL_RenderDrawLine(renderer, MIN_X, MAX_Y, MAX_X, MAX_Y);
			SDL_RenderDrawLine(renderer, MIN_X, MAX_Y, MIN_X, MIN_Y);
			SDL_RenderDrawLine(renderer, MAX_X, MAX_Y, MAX_X, MIN_Y);

					
			//Dibujo los obstaculos
			lista_iterar(lista, dibujar_obstaculo, renderer);


			// Dibujo el recuperador
			obstaculo_dibujar(renderer, recuperador);


			// Dibujo la trayectoria de la bola
			if (! cayendo){
				SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0x00);

				componentes_t *trayectoria = malloc(sizeof(componentes_t) * 100);
				if(trayectoria == NULL){
					fallo_algo = true;
					break;
				}
				trayectoria[0].x = cx;
				trayectoria[0].y = cy;
				trayectoria[0].vx = vx;
				trayectoria[0].vy = vy;

				for(size_t i = 0; i < 100 -1; i++){
					trayectoria[i+1].vy = computar_velocidad(trayectoria[i].vy, G, DT);
					trayectoria[i+1].vx = trayectoria[i].vx * ROZAMIENTO;
					trayectoria[i+1].vy = trayectoria[i+1].vy * ROZAMIENTO;
					trayectoria[i+1].x = computar_posicion(trayectoria[i].x, trayectoria[i+1].vx, DT);
					trayectoria[i+1].y = computar_posicion(trayectoria[i].y, trayectoria[i+1].vy, DT);

					if(i % 3 == 0)
						SDL_RenderDrawLine(renderer, trayectoria[i].x, trayectoria[i].y, trayectoria[i+1].x, trayectoria[i+1].y);
				}
				free(trayectoria);
			}
			
			poligono_destruir(bola);

			// END código del alumno

			SDL_RenderPresent(renderer);
			ticks = SDL_GetTicks() - ticks;
			if(dormir) {
				SDL_Delay(dormir);
				dormir = 0;
			}
			else if(ticks < 1000 / JUEGO_FPS)
				SDL_Delay(1000 / JUEGO_FPS - ticks);
			ticks = SDL_GetTicks();
		}
		
		// Libero la memoria del clon
		lista_destruir(lista, destruir_obstaculo);

		// Libero la memoria del recuperador
		obstaculo_destruir(recuperador);

		// Si salio de la iteracion porque fallo algo o se quiere salir, libero la lista original antes de salir
		if(fallo_algo || event.type == SDL_QUIT){
			lista_destruir(lista_original, destruir_obstaculo);
			break; 
		} 


		// Al final de cada nivel sumo los puntos y destruyo la lista original.
		if(! reiniciar_nivel){		
			lista_destruir(lista_original, destruir_obstaculo);

			puntaje_total += puntaje_nivel;
			sprintf(puntaje_total_text, "Puntaje total: %ld", puntaje_total);

			// Transicion al proximo nivel
			size_t puntos_aux = puntaje_nivel;

			sprintf(puntaje_total_text, "Puntaje total: %ld", puntaje_total - puntos_aux);
			sprintf(texto_puntaje_nivel, "Puntaje nivel: %ld", puntaje_nivel - puntos_aux);

			SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
			SDL_RenderClear(renderer);

	#ifdef TTF
				escribir_texto(renderer, font, texto_puntaje_nivel, 100, 100, BLANCO);
				escribir_texto(renderer, font, puntaje_total_text, 100, 130, BLANCO);
	#endif
			SDL_RenderPresent(renderer);
			SDL_Delay(1000);

			while(puntaje_nivel > 0) {

				if(SDL_PollEvent(&event)){
					if(event.type == SDL_MOUSEBUTTONDOWN)
						break;

					if (event.type == SDL_QUIT)
						break;
				}

				SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
				SDL_RenderClear(renderer);
				SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);
				
				puntos_aux -= 10;

				sprintf(puntaje_total_text, "Puntaje total: %ld", puntaje_total - puntos_aux);
				sprintf(texto_puntaje_nivel, "Puntaje nivel: %ld", puntaje_nivel - puntos_aux);
#ifdef TTF
				escribir_texto(renderer, font, texto_puntaje_nivel, 100, 100, BLANCO);
				escribir_texto(renderer, font, puntaje_total_text, 100, 130, BLANCO);
#endif
				SDL_RenderPresent(renderer);
			}
			if(event.type == SDL_QUIT)
				break;

			SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
			SDL_RenderClear(renderer);
			SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);
			
			sprintf(puntaje_total_text, "Puntaje total: %ld", puntaje_total);
			sprintf(texto_puntaje_nivel, "Puntaje nivel: %ld", puntaje_nivel);

#ifdef TTF
				escribir_texto(renderer, font, texto_puntaje_nivel, 100, 100, BLANCO);
				escribir_texto(renderer, font, puntaje_total_text, 100, 130, BLANCO);
#endif
			SDL_RenderPresent(renderer);

			SDL_Delay(1000);
		}
	}
   

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

#ifdef TTF
    TTF_CloseFont(font);
	TTF_CloseFont(font_chiquito);
    TTF_Quit();
#endif
    SDL_Quit();
	fclose(f);

	if(fallo_algo) return 1;
	
    return 0;
}

