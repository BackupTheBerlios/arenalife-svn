#ifndef _CELULA_H
#define _CELULA_H

#include <pthread.h>
#include <semaphore.h>

#include "cpu.h"

typedef struct organismo organismo; 

struct organismo {
	int id;
	int slices;
	int size;

	int alive;
	int indep;
	int ronda_hijos;

	int mem; // espacio en memoria

	long long int pvida;
	int running;

	struct cpu *pcpu;
	
	organismo *hijo;
	organismo *padre;
	
	int (*load_from_file)(organismo*, char *);
	int (*load_from_bytes)(organismo *, char *insts, int len);
	void * (*vivir_thread)(void *);
	char* (*get_genoma)(organismo *);
	void (*die)(organismo *);
	int (*save)(organismo *);
		
};

int save(organismo *);
int put_in_soup(organismo *);
int are_clones(char *, char *, int);
void * vivir_thread(void *);
void die(organismo *);
organismo* organismo_new(void);
int load_from_file(organismo *, char *);
int load_from_bytes (organismo *, char *, int);
char * get_genoma(organismo *);


#endif
