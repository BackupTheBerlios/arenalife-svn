#include <stdlib.h>
#include <glib.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include "estadistica.h"
#include "globals.h"

struct estadistica *pthis = NULL;

GList *clases = NULL;
GList *errores = NULL;

static int clase_comp (gconstpointer elem, gconstpointer clase) {
	struct clase_cel *pclase = (struct clase_cel *)elem;
	int c = (int)clase;
	//printf("clase: %d\n", pclase->genome_size);
	//printf("elem: %d\n\n", c);
	if (pclase->genome_size == c)
		return 0;
	else
		return 1;
}

/* Ordeno por proporcion ocupada en memoria */
static gint sort_func (gconstpointer a, gconstpointer b) {
	 struct clase_cel *pa, *pb;
	pa = (struct clase_cel *)a;
	 pb = (struct clase_cel *)b;
	if (pa->cantidad*pa->genome_size > pb->cantidad*pb->genome_size)
	              return -1;
	else if (pa->cantidad*pa->genome_size < pb->cantidad*pb->genome_size)
	              return 1;
	else
	              return 0;
}

void REC_relist(void) {
	clases = g_list_sort(clases, sort_func);
}

pthread_mutex_t mutex1;
void REC_cel_creada(int clase) {
	GList *plist;
	struct clase_cel *pclase;
	pthread_mutex_lock(&mutex1);
	pthis->ncels++;
	pthis->inc_cels++;
	if ((plist = g_list_find_custom(clases, (gpointer)clase, clase_comp))) {
		pclase = (struct clase_cel *)plist->data;
		pclase->cantidad++;
	}
	else {
	
		/*if (g_list_length(clases)>10) {
			pclase = (struct clase_cel*)((g_list_last(clases))->data);
			//if (pclase->cantidad==0) {
				clases = g_list_remove(clases, pclase);
				pthread_mutex_unlock(&mutex1);
				return;
			//}
			pclase = NULL;
		}*/
		pclase = (clase_cel*)malloc(sizeof(*pclase));
		pclase->genome_size = clase;
		pclase->cantidad=1;
		clases = g_list_append(clases, pclase);
		//clases = g_list_insert_sorted(clases, pclase, sort_func);
	}
	//printf("clase %d, cant: %d\n",pclase->genome_size, pclase->cantidad);
	pthread_mutex_unlock(&mutex1);
}

pthread_mutex_t mutex2;
void REC_cel_eliminada(int clase){
	GList *plist;
	struct clase_cel *pclase;
	pthread_mutex_lock(&mutex2);
	if ((plist = g_list_find_custom(clases, (gpointer)clase, clase_comp))) {
		pclase = (struct clase_cel *)plist->data;
		pclase->cantidad--;
		if (pclase->cantidad<0) {
			//printf("clase %d, cant: %d\n",pclase->genome_size, pclase->cantidad);
			assert(pclase->cantidad+1);
			//pclase->cantidad=0;
		}
	}
	pthis->dec_cels++;
	pthis->ncels--;
	pthread_mutex_unlock(&mutex2);
}

int get_cant_clase(int clase) {
	GList *plist;
	struct clase_cel *pclase;
	if ((plist = g_list_find_custom(clases, (gpointer)clase, clase_comp))) {
		return pclase->cantidad;	
	}
	else
		return 0;
}

void REC_saved_genoma(char *genoma) {
	memset(pthis->last_saved_genoma,0,20);
	strncpy(pthis->last_saved_genoma,genoma,20);
}

GList* get_clases(void) {
	return clases;
}

GList* get_errores(void) {
	return errores;
}

static int error_comp (gconstpointer elem, gconstpointer clase) {
	struct clase_error *pclase = (struct clase_error *)elem;
	int c = (int)clase;
	//printf("clase: %d\n", pclase->genome_size);
	//printf("elem: %d\n\n", c);
	if (pclase->tipo == c)
		return 0;
	else
		return 1;
}

pthread_mutex_t mutex3;
void REC_error(int tipo) {
	GList *plist;
	struct clase_error *pclase;
	pthread_mutex_lock(&mutex3);
	pthis->seg_faults++;
	if ((plist = g_list_find_custom(errores, (gpointer)tipo, error_comp))) {
		pclase = (struct clase_error *)plist->data;
		pclase->cantidad++;
	}
	else {
		pclase = (struct clase_error*)malloc(sizeof(*pclase));
		pclase->tipo = tipo;
		pclase->cantidad=1;
		errores = g_list_append(errores, pclase);
	}
	pthread_mutex_unlock(&mutex3);
}

pthread_mutex_t mutex4;
void REC_memory_free(int total) {
	pthread_mutex_lock(&mutex4);
	pthis->memfree = total;
	pthread_mutex_unlock(&mutex4);
}

pthread_mutex_t mutex5;
void REC_memory_total(int total) {
	pthread_mutex_lock(&mutex5);
	pthis->memtotal = total;
	pthread_mutex_unlock(&mutex5);
}

pthread_mutex_t mutex6;
void REC_instexec(void){
	pthread_mutex_lock(&mutex6);
	pthis->instexec++;
	pthis->instexec_total++;
	pthread_mutex_unlock(&mutex6);
}
void REC_reset(void){
	if (pthis==NULL) pthis = (estadistica*)malloc(sizeof(*pthis));
	pthis->last_dom = NULL;
	pthis->last_dom_cont = 0;
	pthis->instexec_total=0;
	pthis->ncels = 0;
	pthis->seg_faults = 0;
	pthis->generacion=0;
	memset(pthis->last_saved_genoma,0,20);
	REC_reset_ronda();
}

/* int to str */
static char *itos(int val) {
      char *str;
      int i, len = 0;
      if(val == 0) {
	      str = (char*)malloc(2);
	      *str = '0';
	      *(str+1) = '\0';
	      return(str);
      }
      i = val;

      for(i=val; i /= 10; len++);

      str = (char*)malloc(len + 1);
      str += len+1;

      do {
	      *--str = (val % 10) + '0';
	      //*(str++) = (val % 10) + '0';
	      //*(str++) = '0';
      } while(val /= 10);

      str[++len] = '\0';

      return(str);
}

//hola
//holas

static void IO_save_dom(GList *doms, int lastdom_cont) {
	int filefd;
	int i;
	int prop_i;
	clase_cel *dom;
	char *generaciones, *lastdom_s;
	char *size, *prop_s, *gen_s;	
	if ((filefd=open("dominantes", O_CREAT|O_WRONLY|O_APPEND,0666)) == -1) {
		perror("open");
		assert(0);
		return;
	}
	
	if (lastdom_cont>1) {
		lastdom_s = itos(pthis->last_dom->genome_size);
		generaciones = itos(lastdom_cont);
		write(filefd, "# Clase ", 8);
		write(filefd, lastdom_s, strlen(lastdom_s));
		write(filefd, " domino por ", 12);
		write(filefd, generaciones, strlen(generaciones));
		write(filefd, " generaciones\n", 14);
		free(generaciones);
		free(lastdom_s);	
	}
	
	gen_s=itos(pthis->generacion);
	write(filefd, gen_s, strlen(gen_s));
	for (i=0;i<(int)g_list_length(doms);i++) {
		dom = (clase_cel*)g_list_nth_data(doms,i);
		prop_i=(int)(100.0*(((float)(dom->genome_size*dom->cantidad))/(float)pthis->memtotal));
		//prop_i = dom->cantidad;	
		if (prop_i==0) prop_i=1;
		size = itos(dom->genome_size);
		prop_s = itos(prop_i);
		write (filefd, " ",1);
		write(filefd, size, strlen(size));
		write (filefd, " ",1);
		write(filefd, prop_s, strlen(prop_s));
		free(size);
		free(prop_s);	
	}
	write(filefd, "\n", 1);
	free(gen_s);
	close(filefd);
}

static void REC_save_dominante(void) {
	int i;
	GList *doms = NULL;
	gpointer t = NULL;
	int cont = pthis->last_dom_cont;
	for (i=0;i<N_DOMINANTES;i++) {
		t = g_list_nth_data(clases,i);
		if (t!=NULL)
			doms = g_list_append(doms,t);
		t=NULL;
	}
	if (g_list_nth_data(clases,0)==pthis->last_dom) {
		pthis->last_dom_cont++;
		cont=0; //mientras se mantenga el lider, no guardo.
	} else { /*cambio de lider*/
		pthis->last_dom_cont=1;
	}
	IO_save_dom(doms, cont);
	pthis->last_dom = (clase_cel*)g_list_nth_data(clases, 0);
}

void REC_reset_ronda(void){
	pthis->memfree = 0;
	if (pthis->instexec_total > INST_PER_GEN) {
		pthis->instexec_total=pthis->instexec_total % INST_PER_GEN;
		REC_relist();
		REC_save_dominante();
		pthis->generacion++;
		//Benchmark: time ./arena -> 16 segundos (4 gens)
		//if (pthis->generacion==4)
		//	assert(0);
	}
	pthis->instexec = 0;
	pthis->inc_cels = 0;
	pthis->dec_cels = 0;
}

struct estadistica* estadistica_get(void) {
	return pthis;
}
