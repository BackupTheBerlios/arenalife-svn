#ifndef _SLICER_H
#define _SLICER_H

#include <pthread.h>
#include <semaphore.h>
#include <glib.h>
#include "celula.h"
#include "globals.h"

typedef struct slicer slicer;

struct slicer {
	pthread_t tid[NHILOS];
	int cont_hilos;
	
	pthread_mutex_t ronda_mutex;
	pthread_mutex_t wait_m;
	pthread_cond_t wait_c;
	
	int ncels;
	unsigned long long int idsacum;
	int inst_exec; //instrucciones ejecutadas
		
	void (*agregar_organismo) (celula*);	
	celula* (*create_celula_from_file)(char*);
	celula* (*create_celula_from_bytes)(char*, int);
	int (*crear_hijo)(celula*);
	int (*remover_organismo)(celula *pcel);
	int (*create_cel_id)(void);
	int (*get_cel_slices)(celula*);
	int (*give_slice)(void);
	int (*reaper)(void);
	void (*relist)(void);
	int (*get_total)(void);
};

/* Metodos Privados */
void agregar_organismo(celula*);
int remover_organismo(celula *pcel);
int get_cel_slices(celula*);

/* Metodos Publicos */
slicer* slicer_get(void);
void relist(void);
int reaper(void);
int get_total(void);
celula* create_celula_from_file(char*);
celula * create_celula_from_bytes(char *, int);
int crear_hijo(celula*);
int give_slice(void);
#endif
