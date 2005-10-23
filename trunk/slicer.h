#ifndef _SLICER_H
#define _SLICER_H

#include <pthread.h>
#include <semaphore.h>
#include <glib.h>
#include "organismo.h"
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
		
	void (*agregar_organismo) (organismo*);	
	organismo* (*organismo_desde_archivo_new)(char*);
	organismo* (*organismo_desde_bytes_new)(char*, int);
	int (*crear_hijo)(organismo*);
	int (*remover_organismo)(organismo *pcel);
	int (*create_cel_id)(void);
	int (*get_cel_slices)(organismo*);
	int (*correr_ronda)(void);
	int (*reaper)(void);
	void (*relist)(void);
	int (*get_total)(void);
};

/* Metodos Privados */
void agregar_organismo(organismo*);
int remover_organismo(organismo *pcel);
int get_cel_slices(organismo*);

/* Metodos Publicos */
slicer* slicer_get(void);
void relist(void);
int reaper(void);
int get_total(void);
organismo* organismo_desde_archivo_new(char*);
organismo * organismo_desde_bytes_new(char *, int);
int crear_hijo(organismo*);
int correr_ronda(void);
#endif
