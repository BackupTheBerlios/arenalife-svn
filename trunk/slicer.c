#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <assert.h>
#include <pthread.h>
#include <math.h>
#include <semaphore.h>
#include "slicer.h"
#include "estadistica.h"
#include "globals.h"

#define DEBUG_SLI

static GList *slicer_list = NULL;
static GSList *reaper_list = NULL;

static slicer *the_slicer = NULL; //no puedo hacer extern ... desde otro modulo

/* para asignar pvida a las organismos */
static unsigned long long int vidacont=0;

static pthread_mutex_t acum;
static int create_cel_id(slicer *pthis) {
	int newid;
	pthread_mutex_lock(&acum);
	newid = pthis->idsacum++;
	pthread_mutex_unlock(&acum);
	return newid;
}

static void slicer_const(slicer *pthis) {
	pthis->agregar_organismo = &agregar_organismo;
	pthis->organismo_desde_archivo_new = &organismo_desde_archivo_new;
	pthis->organismo_desde_bytes_new = &organismo_desde_bytes_new;
	pthis->crear_hijo = &crear_hijo;
	pthis->remover_organismo = remover_organismo;	
	pthis->get_cel_slices = &get_cel_slices;
	pthis->correr_ronda = &correr_ronda;
	pthis->get_total = &get_total;
	pthis->reaper = &reaper;
	pthis->relist = &relist;
	pthis->ncels = 0;
	pthis->inst_exec = 0;
	pthis->cont_hilos = 0;
	pthis->idsacum = 0;
}

/* singleton */
slicer* slicer_get(void) {
	if (the_slicer == NULL) {
		the_slicer = malloc(sizeof(*the_slicer));
		slicer_const(the_slicer);
	}
	return the_slicer;		
}

int remover_organismo(organismo *pcel) {
	slicer *pthis = slicer_get();
	if (g_list_find(slicer_list, pcel)) {
		slicer_list=g_list_remove(slicer_list, pcel);
		reaper_list=g_slist_remove(reaper_list, pcel);
		pcel->die(pcel);
		REC_cel_eliminada(pcel->size);
	} 
	else
		assert(0);
	
	pthis->ncels--;
	return 1;
}


static organismo* next_tobe_cleaned (void) {
	organismo *pcel=NULL;
	//pcel = g_slist_nth_data(reaper_list,0);
	
	// por ser el 1er elemento
	pcel = reaper_list->data;

	if (pcel == NULL) {
		return NULL; //no more cels
	}
	return pcel;
}

int reaper(void) {
	slicer *pthis = slicer_get();
	organismo *pcel = NULL;
	pcel = next_tobe_cleaned();
	if (pcel) {
		organismo *phijo = pcel->hijo;
		/* si maloque para un hijo pero este todavia no es independiente, me lo llevo */
		if (phijo && phijo->indep==0) {
			phijo->die(phijo);
		}
		pthis->remover_organismo(pcel);
	}
	else
		assert(0);
	return 1;
}

int get_total() {
	return g_list_length(slicer_list);
}


static pthread_mutex_t crear_hijo_f;
int crear_hijo(organismo *pcel) {
	slicer *pthis = slicer_get();
	pcel->id = create_cel_id(pthis);
	pcel->indep=1;
	pcel->slices = pthis->get_cel_slices(pcel);
	pthread_mutex_lock(&crear_hijo_f);
	pcel->pvida = vidacont++;
	pthis->ncels++; 
	pthread_mutex_unlock(&crear_hijo_f);
	
	/* guarda genoma en disco */
	pcel->save(pcel);

	/* agrega a la lista del slicer y reaper */	
	pthis->agregar_organismo(pcel);
	return 1;
}

organismo * organismo_desde_bytes_new(char *genoma, int len) {
	slicer *pthis = slicer_get();
	organismo *pcel = organismo_new();
	
	//evito inyectar mientras el modulo arena prepara todo para otra ronda
	pthread_mutex_lock(&pthis->ronda_mutex);	
	
	if(pcel->load_from_bytes(pcel, genoma, len)) {
		pcel->indep=1;
		pcel->id = create_cel_id(pthis);
		pcel->pvida = vidacont++;	
		pcel->slices = pthis->get_cel_slices(pcel);
		pthis->ncels++; 
		pthis->agregar_organismo(pcel);
	}
	else {
		pcel->die(pcel);
		pthread_mutex_unlock(&pthis->ronda_mutex);	
		return NULL;
	}
	pthread_mutex_unlock(&pthis->ronda_mutex);	
	return pcel;
}


organismo * organismo_desde_archivo_new(char *filename) {
	slicer *pthis = slicer_get();
	organismo *pcel = organismo_new();
	
	if(pcel->load_from_file(pcel, filename)) {
		pcel->indep=1;
		pcel->id = create_cel_id(pthis);
		pcel->pvida = vidacont++;	
		pcel->slices = pthis->get_cel_slices(pcel);
		pthis->ncels++; 
		pthis->agregar_organismo(pcel);
	}
	else {
		pcel->die(pcel);
		return NULL;
	}
	return pcel;
}

#ifdef MT
static void wait_threads(void) {
	int i;
	slicer *pthis = slicer_get();
	for (i=0;i<pthis->cont_hilos;i++)
		pthread_join(pthis->tid[i],NULL);
	pthis->cont_hilos=0;
}
#endif

/* para cada organismo en la ronda ... */
static void cel_time(gpointer c, gpointer nada) {
	organismo *pcel = (organismo *)c;
	pcel->ronda_hijos=0;
#ifdef MT
	slicer *pthis = slicer_get();
        if (pthread_create(&pthis->tid[pthis->cont_hilos], NULL, vivir_thread, pcel)!=0) {
		assert(0);
	}
	
	pthis->cont_hilos++;
	
	//espero los hilos que lance si llegue a mi maximo de hilos
	if (pthis->cont_hilos == NHILOS)
	       wait_threads();	
#else
	/* Single Thread */	
	vivir_thread(pcel);
#endif
}

int correr_ronda() {
	slicer *pthis = slicer_get();

	/* si no hay organismos, espero a que alguien inyecte */
	if (g_list_length(slicer_list)<1) {
		pthread_mutex_lock(&pthis->wait_m);
		pthread_cond_wait(&pthis->wait_c, &pthis->wait_m);
		pthread_mutex_unlock(&pthis->wait_m);
	}

	pthis->cont_hilos=0;
       
        // para cada organismo...	
	g_list_foreach(slicer_list, cel_time, NULL);
#ifdef MT
	// si hay hilos corriendo...
	if (pthis->cont_hilos > 0)
		wait_threads();

	// si hay por nacer...
	//g_slist_foreach(nacidos_list,born, NULL);
	//g_slist_free(nacidos_list);
	//nacidos_list=NULL;
#endif
	return 1;	
}


int get_cel_slices(organismo* pcel) {
	// exp > 1 BENEFICIA A LAS CRIATURAS MAS GRANDES
	// exp < 1 BENEFICIA A LAS CRIATURAS MAS CHICAS
	// exp = 1 NEUTRAL
	int slices;
	double exp = 0.7;

	slices= (int)powf((double)pcel->size, exp);
	
	//slices=20;
	
	//slices = pcel->size;

	if (slices<1)
		slices=1;	
	return slices;
}

static gint pvida_comp (gconstpointer a, gconstpointer b) {
	organismo *pa, *pb;
	pa = (organismo *)a;
	pb = (organismo *)b;
	if (pa->pvida < pb->pvida)
		return -1;
	else if (pa->pvida > pb->pvida)
		return 1;
	else
		return 0;
}

/* las organismos que produjeron algun error o vivieron mucho reducen su pvida,
 *  por eso debo relistar para cambiar el orden */
void relist(void) {
	reaper_list = g_slist_sort(reaper_list, pvida_comp);
}

pthread_mutex_t add_f;
void agregar_organismo (organismo* cel) {
	assert(cel->size);
	REC_cel_creada(cel->size);
	
	GList *lpadre = NULL;
	pthread_mutex_lock(&add_f);
	lpadre = g_list_find(slicer_list, cel->padre);
	if (lpadre==NULL) {
		slicer_list= g_list_append(slicer_list,cel);
	} else	
		slicer_list=g_list_insert_before(slicer_list, lpadre ,cel);
	reaper_list= g_slist_append(reaper_list,cel);
	pthread_mutex_unlock(&add_f);
}


