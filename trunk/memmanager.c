#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "memmanager.h"
#include "estadistica.h"
#include "mutation.h"

static memmanager *the_mem = NULL;


static void memmanager_const(memmanager *pthis) {
	pthis->show_free_heap = &show_free_heap;
	pthis->show_used_heap = &show_used_heap;
	pthis->assert_mem = &assert_mem;
	pthis->defrag = &defrag;
	pthis->load_cel = &load_cel;
	pthis->soup = (char*)malloc(SOUP_SIZE+1);
	memset(pthis->soup, 0, SOUP_SIZE+1);
	pthis->soup_mutate = &soup_mutate;
	pthis->Vmalloc = &Vmalloc;
	pthis->Vfree = &Vfree;
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

void memmanager_reset(void) {
	if (the_mem != NULL) free(the_mem);
	the_mem = NULL;
}

void init_heap () {
	memmanager *pthis = memmanager_get();
	segment *free = segment_new(0, SOUP_SIZE);
	pthis->free_heap = g_slist_append(pthis->free_heap, free);	
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

/* busca segmentos contiguos y los une en 1 solo segmento */
void defrag() {
	memmanager *pthis = memmanager_get();
	int i, total;
	segment *pseg, *psegnext;
	
	//pthis->free_heap = g_slist_sort(pthis->free_heap, seg_comp);

	total = g_slist_length(pthis->free_heap);
	for (i=0;i<total-1;i++) {
		pseg=(segment *)g_slist_nth_data(pthis->free_heap, i);
		psegnext=(segment *)g_slist_nth_data(pthis->free_heap, i+1);
		if ((pseg->fin+1) == psegnext->inicio) {
			pseg->fin = psegnext->fin;
			pseg->size += psegnext->size;
		       	pthis->free_heap = g_slist_remove(pthis->free_heap, psegnext);	
			total-=1;
			free(psegnext);
		}
	}
}

static int segment_find (gconstpointer elem, gconstpointer val) {
	segment *pseg = (segment*)elem;
	unsigned int inicio = (unsigned int) val;
	if (pseg->inicio == inicio)
		return 0;
	else
		return 1;	
}

void Vfree(unsigned int mem) {
	memmanager *pthis = memmanager_get();
	segment *pseg = 0;
	GSList *l = 0;
	if (mem >= 0) {
		l = g_slist_find_custom( pthis->used_heap, (gpointer)mem, segment_find); 
		if (l) {
			pseg = (segment*)l->data;
			pthis->free+=(pseg)->size;
			pthis->free_heap = g_slist_insert_sorted(pthis->free_heap, pseg,seg_comp);
			pthis->used_heap = g_slist_remove(pthis->used_heap, pseg);
		}
	}
	else {
		REC_error(SEG_NULL);
		return;
	}
	REC_memory_free(total_free());
}

int Vmalloc(unsigned int csize) {
	memmanager *pthis = memmanager_get();
	segment *freeseg = NULL;
	segment *newseg;
	
	
	if (g_slist_length(pthis->free_heap)==0) { 
		return -1;
	}

	/* FIRST FIT */
	
	if ((freeseg = segment_fit_search(csize))) {
	
		newseg = segment_new(freeseg->inicio, csize);
		
		segment_resize(freeseg, csize);
		
		pthis->used_heap = g_slist_append(pthis->used_heap,newseg);
	
		pthis->free -= newseg->size;

		//asigno mem a organismo
		if (!newseg) REC_error(MAL_ERROR);
	
		REC_memory_free(total_free());
		return newseg->inicio;
	
	} else {
		REC_error(MEM_FULL);
		return -1;	
	}
	
}

static void assert_seg(gpointer p, gpointer u_data) {	
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

static void show_heap(GSList *l) {
	g_slist_foreach(l, print_elem, stdout);	
}

void assert_mem() {
	memmanager *pthis = memmanager_get();
	g_slist_foreach(pthis->free_heap, assert_seg, stdout);	
	g_slist_foreach(pthis->used_heap, assert_seg, stdout);		
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


/* carga las instrucciones al inicio del segmento de la organismo */
int load_cel(char *coded_insts, organismo *pcel) {
	memmanager *mman = memmanager_get();
	memcpy(mman->soup + pcel->mem, coded_insts, pcel->size);
	pcel->pcpu->ip = pcel->mem;
	return 1;
}

/* autorizo a escribir solo si:
 * 1 - Es el espacio de la misma organismo 
 * 2 - Es el espacio del hijo todavia no independiente
 * */
int auth_w(cpu *pcpu) {
	int seg;
	int fin;
	organismo *pcel = pcpu->pcel;	
	
	if ((pcpu->bx >= pcel->mem) && (pcpu->bx <= pcel->mem + pcel->size)) {
		return 1;
	}
	if (pcel->hijo) {
		if ((pcel->hijo)->indep == 0) {
			seg = ((pcel->hijo)->mem);
			fin = seg + ((pcel->hijo)->size);
			if (pcpu->ax >= seg && pcpu->ax <= fin)
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
	organismo *pcel = pcpu->pcel;
	if ((pcpu->ip >= pcel->mem) && (pcpu->ip <= pcel->mem + pcel->size)) {
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

segment* segment_new(int inicio, int size) {
	segment *pthis = (segment*)malloc(sizeof(*pthis));
	pthis->inicio = inicio;
	pthis->size = size;
	pthis->fin = inicio + size - 1;
	return pthis;
}

static int segment_fit (gconstpointer elem, gconstpointer val) {
	segment *pseg = (segment*)elem;
	unsigned int size = (unsigned int) val;
	if (pseg->size >= size)
		return 0;
	else
		return 1;	
}

segment* segment_fit_search(unsigned int size) {
	segment *pseg = 0;
	GSList *l = NULL;
	memmanager *mman = memmanager_get();
	
	l = g_slist_find_custom(mman->free_heap, (gpointer)size, segment_fit);
	if (l)
		pseg = (segment*)l->data;
	
	return pseg;

}

void segment_resize(segment *pseg, int size) {
	memmanager *pthis = memmanager_get();
	pseg->size -= size;
	
	if (pseg->size == 0) {
		pthis->free_heap = g_slist_remove(pthis->free_heap, pseg);
		free(pseg);
		return;
	}	
	pseg->inicio += size; 	
}
