#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "memmanager.h"
#include "estadistica.h"
#include "mutation.h"

static memmanager *the_mem = NULL;

#define SOUP_SIZE 70000 //bytes
#define PROP_FREE 30 //porcentaje que el cleaner mantiene libre en la soup

static void memmanager_const(memmanager *pthis) {
	pthis->show_free_heap = &show_free_heap;
	pthis->show_used_heap = &show_used_heap;
	pthis->assert_mem = &assert_mem;
	pthis->defrag = &defrag;
	pthis->load_cel = &load_cel;
	pthis->soup = (char*)malloc(SOUP_SIZE+1);
	memset(pthis->soup, 0, SOUP_SIZE+1);
	pthis->soup_mutate = &soup_mutate;
	pthis->malocar = &malocar;
	pthis->liberar = &liberar;
	pthis->total_free = &total_free;
	pthis->init_heap = &init_heap;
	pthis->enough_free = &enough_free;
	pthis->free_heap = NULL;
	pthis->used_heap = NULL;
	pthis->total = SOUP_SIZE;
	pthis->free = SOUP_SIZE;
	REC_memory_total(SOUP_SIZE);
}

memmanager* memmanager_get(void) {
	if (the_mem == NULL) {
		the_mem = (memmanager*)malloc(sizeof(*the_mem));
		memmanager_const(the_mem);	
		the_mem->init_heap();
	}
	return the_mem;
}

void init_heap () {
	memmanager *pthis = memmanager_get();
	segment *free = (segment*)malloc(sizeof(*free));
	free->inicio = 0;
	free->fin = SOUP_SIZE-1;
	free->size = SOUP_SIZE;
	pthis->free_heap = g_list_append(pthis->free_heap, free);	
}

static gint seg_comp (gconstpointer a, gconstpointer b) {
	segment *pa, *pb;
	pa = (segment *)a;
	pb = (segment *)b;
	if (pa->inicio > pb->inicio)
		return 1;
	else if (pa->inicio < pb->inicio)
		return -1;
	else
		return 0;
}

int total_free() {
	memmanager *pthis = memmanager_get();
	return pthis->free;
}

int memsize (void) {
	return SOUP_SIZE;
}

int enough_free() {
	memmanager *pthis = memmanager_get();
	if (pthis->free >= (((float)PROP_FREE)/100.0)*(float)SOUP_SIZE)
		return 1;
	else
		return 0;	
}

/*
struct limits {
	int inf;
	int sup;
};

static void print_elem(gpointer p, gpointer u_data);

gint inter_find(gconstpointer a, gconstpointer userdata) {
	struct limits *lims = (struct limits*)userdata;
	segment *seg = (segment *)a;
	// si encuentro un hueco...
	//printf("lim inf:%d\n",lims->inf);
	//printf("lim sup:%d\n\n",lims->sup);
	if (lims->inf < seg->inicio && lims->sup > seg->inicio)
		return 0;
	else	
		return 1;
}
*/

/* busca segmentos contiguos y los une en 1 solo segmento */
void defrag() {
	memmanager *pthis = memmanager_get();
	int i, total;
	segment *pseg, *psegnext;
	
	pthis->free_heap = g_list_sort(pthis->free_heap, seg_comp);

	total = g_list_length(pthis->free_heap);
	for (i=0;i<total-1;i++) {
		pseg=(segment *)g_list_nth_data(pthis->free_heap, i);
		psegnext=(segment *)g_list_nth_data(pthis->free_heap, i+1);
		if ((pseg->fin+1) == psegnext->inicio) {
			pseg->fin = psegnext->fin;
			pseg->size += psegnext->size;
		       	pthis->free_heap = g_list_remove(pthis->free_heap, psegnext);	
			total-=1;
			free(psegnext);
		} /*else {
			struct limits l;
			l.inf = (pseg->fin);
			l.sup = (psegnext->inicio);
			// si hay hueco, uno los segmentos 
			if (!g_list_find_custom(pthis->used_heap, (gpointer)&l,inter_find)) {
				//print_elem(pseg, stdout);
				//print_elem(psegnext, stdout);
				pseg->size += psegnext->size;
				pseg->fin +=psegnext->size;
				//print_elem(pseg, stdout);
		       		pthis->free_heap = g_list_remove(pthis->free_heap, psegnext);	
				//assert(0);
			}
			//free(&l);	
		}*/
	}
}

int liberar(celula *pcel) {
	memmanager *pthis = memmanager_get();
	if (pcel->mem!=NULL) {
		pthis->free+=(pcel->mem)->size;
		pthis->free_heap = g_list_insert_sorted(pthis->free_heap, pcel->mem,seg_comp);
		pthis->used_heap = g_list_remove(pthis->used_heap, pcel->mem);
	}
	else {
		REC_error(SEG_NULL);
	}
	REC_memory_free(total_free());
	return 1;
}

/* Se mantiene una lista enlazada con los segmentos libres y otra con los segmentos ocupados. 
 * Devuelve 0 si no hay lugar, 1 si maloco bien. */
static pthread_mutex_t maloc;
int malocar(celula *pcel) {
	memmanager *pthis = memmanager_get();
	int i;
	GList *l = NULL;
	segment *freeseg = NULL;
	segment *newseg;
	

	//MUTEX LOCK
	pthread_mutex_lock(&maloc);
	
	/* tomo el primer segmento libre */
	if (g_list_length(pthis->free_heap)==0) { 
		pthread_mutex_unlock(&maloc);
		return 0;
	}


	/* FIRST FIT */
	l = g_list_nth(pthis->free_heap, 0);
	freeseg = (segment *)l->data; 
	
	/* sigo buscando sino encuentro un segmento donde quepa la celula */
	for (i=1; (pcel->size > freeseg->size) ;i++) {
		l = g_list_nth(pthis->free_heap, i);
		if (l==NULL) break;
		freeseg = (segment *)l->data; 
	}

	/* si no encontre un segmento con espacio libre, retorno */
	if (pcel->size > freeseg->size) {
		REC_error(MEM_FULL);
		//MUTEX UNLOCK
		pthread_mutex_unlock(&maloc);
		return 0;
	}
	
	newseg = (segment*)malloc(sizeof(*newseg));
	newseg->inicio = freeseg->inicio;
	newseg->fin = newseg->inicio + pcel->size - 1; /*0+80-1=79*/
	newseg->size = pcel->size;
	
	freeseg->size -= pcel->size;
	
	if (freeseg->size == 0) {
		pthis->free_heap = g_list_remove(pthis->free_heap, freeseg);
		free(freeseg);
	}	
	if (freeseg->size < 0) {
		REC_error(MEM_DESBORD);
		assert(0);
	}	
	freeseg->inicio += pcel->size; 	
		
	pthis->used_heap = g_list_append(pthis->used_heap,newseg);
	
	
	pthis->free-=newseg->size;

	//MUTEX UNLOCK
	pthread_mutex_unlock(&maloc);
	
	//asigno mem a celula
	if (!newseg) REC_error(MAL_ERROR);
	pcel->mem=newseg;	
	
	REC_memory_free(total_free());
	return 1;
	
}

static void assert_seg(gpointer p, gpointer u_data) {	
	//if (rand()>(RAND_MAX - RAND_MAX/400000))
	//	ui_drawi(25,3,rand());
	segment *pseg = (segment *)p;
	int a;
	if (pseg) {
		a = pseg->fin - pseg->inicio+1;
		a -= pseg->size;
		if (a!=0) {
			REC_error(ASSERT_SEG);
		}
	}
}


static void print_elem(gpointer p, gpointer u_data) {
        FILE *f = (FILE*)u_data;
	segment *seg;
	seg = (segment *)p; 
	fprintf(f, "Segment data ->  ");	
	fprintf(f, "Inicio:%d\t", seg->inicio);	
	fprintf(f, "Size:%d\t", seg->size);	
	fprintf(f, "Fin:%d\n", seg->fin);
}

static void show_heap(GList *l) {
	g_list_foreach(l, print_elem, stdout);	
}

void assert_mem() {
	memmanager *pthis = memmanager_get();
	g_list_foreach(pthis->free_heap, assert_seg, stdout);	
	g_list_foreach(pthis->used_heap, assert_seg, stdout);		
}

void show_free_heap() {
	memmanager *pthis = memmanager_get();
	fprintf(stdout, "Free Heap:\n");	
	show_heap(pthis->free_heap);
}

void show_used_heap() {
	memmanager *pthis = memmanager_get();
	fprintf(stdout, "Used Heap:\n");	
	show_heap(pthis->used_heap);
}


/* carga las instrucciones al inicio del segmento de la celula */
int load_cel(char *coded_insts, celula *pcel) {
	memmanager *mman = memmanager_get();
	memcpy(mman->soup + (pcel->mem)->inicio, coded_insts, pcel->size);
	pcel->pcpu->ip = (pcel->mem)->inicio;
	return 1;
}

/* autorizo a escribir solo si:
 * 1 - Es el espacio de la misma celula 
 * 2 - Es el espacio del hijo todavia no indepentiente
 * */
int auth_w(cpu *pcpu) {
	segment *seg = NULL;
	celula *pcel = pcpu->pcel;	
	
	if ((pcpu->bx >= (pcel->mem)->inicio) && (pcpu->bx <= (pcel->mem)->fin)) {
		return 1;
	}
	if (pcel->hijo) {
		if ((pcel->hijo)->indep == 0) {
			seg = ((pcel->hijo)->mem);
			if (pcpu->ax >= seg->inicio && pcpu->ax <= seg->fin)
				return 1;
		}
	}
	return -1;
}

/* copia de BX a AX solo si estoy autorizado */
int set_byte(cpu *pcpu) {
	memmanager *mman = memmanager_get();
	
	if ((pcpu->ax<((SOUP_SIZE)) && pcpu->ax >=0) 
	&& (pcpu->bx<(SOUP_SIZE) && pcpu->bx >=0)) {
		if (auth_w(pcpu)>-1) {
			if (COPY_falla()) 
				*(mman->soup+pcpu->ax)=my_rand(0,31);
			else
				*(mman->soup+pcpu->ax)=*(mman->soup+pcpu->bx);
			return 1;
		}
	}	
	
	REC_error(SEG_FAULT);
	return -1;
	
}

/* Solo puedo ejecutar la division si estoy en mi propio espacio */
int auth_divide(cpu *pcpu) {
	celula *pcel = pcpu->pcel;
	if ((pcpu->ip >= (pcel->mem)->inicio) && (pcpu->ip <= (pcel->mem)->fin)) {
		return 1;
	}
	else
		return 0;
}

/* +R para todos en toda mi memoria */
char get_byte(int pos) {
	memmanager *mman = memmanager_get();
	if ((pos<(SOUP_SIZE)) && pos >=0)
		return (char)(*(mman->soup+pos));
	else {
		REC_error(SEG_FAULT);
		return -1;
	}
}

void soup_mutate(void) {
	memmanager *mman = memmanager_get();
	
	if (SOUP_falla()) {
		int pos = my_rand(0, memsize()-1);
		*(mman->soup + pos) = my_rand(0,31);
	}
}
