CFLAGS = -Wall -std=c99 -Werror -pedantic -c
LFLAGS = -lm -lSDL2 -lSDL2_ttf 
CC = gcc

all: peggle

peggle: main.o fisica.o poligono.o obstaculo.o tipos.h leer.o lista.o
	$(CC) main.o fisica.o poligono.o obstaculo.o leer.o lista.o -o peggle -g $(LFLAGS) 

main.o: main.c config.h tipos.h fisica.h poligono.h leer.h obstaculo.h lista.h
	$(CC) $(CFLAGS) main.c -DTTF -g -DTTF

fisica.o: fisica.c fisica.h
	$(CC) $(CFLAGS) fisica.c -g

poligono.o: poligono.c poligono.h tipos.h config.h 
	$(CC) $(CFLAGS) poligono.c -g

leer.o: leer.c leer.h config.h
	$(CC) $(CFLAGS) leer.c -g

obstaculo.o: obstaculo.c obstaculo.h tipos.h poligono.h leer.h config.h
	$(CC) $(CFLAGS) obstaculo.c -g

lista.o: lista.c lista.h
	$(CC) $(CFLAGS) lista.c -g

clean:
	rm -vf *.o

valgrind:
	valgrind --suppressions=suppressions_20211_tp1.supp --leak-check=full ./peggle peggle.bin