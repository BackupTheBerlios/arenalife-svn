#ifndef _CELULA_H
#define _CELULA_H

#include <pthread.h>
#include <semaphore.h>

#include "cpu.h"

typedef struct celula celula; 

struct celula {
	int id;
	int slices;
	int size;

	int alive;
	int indep;
	int ronda_hijos;

	struct segment *mem; //espacio en memoria
	
	long long int pvida;
	int running;

	struct cpu *pcpu;
	
	celula *hijo;
	celula *padre;
	
	int (*load_from_file)(celula*, char *);
	int (*load_from_bytes)(celula *, char *insts, int len);
	void * (*vivir_thread)(void *);
	char* (*get_genoma)(celula *);
	void (*die)(celula *);
	int (*save)(celula *);
		
};

int save(celula *);
int put_in_soup(celula *);
int are_clones(char *, char *, int);
void * vivir_thread(void *);
void die(celula *);
celula* celula_new(void);
int load_from_file(celula *, char *);
int load_from_bytes (celula *, char *, int);
char * get_genoma(celula *);


#endif
