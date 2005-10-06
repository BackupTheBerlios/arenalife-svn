#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include "memmanager.h"
#include "instructor.h"
#include "estadistica.h"
#include "cpu.h"
#include "genbank.h"
#include "mutation.h"
#include "globals.h"
#include "slicer.h"

static void celula_const(celula *cel) {
	cel->load_from_file = &load_from_file;
	cel->load_from_bytes = &load_from_bytes;
	cel->vivir_thread = &vivir_thread;
	cel->die = &die;
	cel->get_genoma = &get_genoma;	
	cel->pcpu = malloc(sizeof(*cel->pcpu));
	memset(cel->pcpu,0,sizeof(*cel->pcpu));
	
	(cel->pcpu)->run = &run;
	
	/* El CPU mantiene una referencia a la celula */
	(cel->pcpu)->pcel = cel;

	cel->save = &save;
	cel->mem = NULL;
	cel->hijo = NULL;
	cel->padre = NULL;
	cel->running=0;
	cel->alive=1;
	cel->indep=0;
	cel->ronda_hijos=0;
}

celula* celula_new(void) {
	celula *pcel = malloc(sizeof(*pcel));
	celula_const(pcel);
	return pcel;
}

void die(celula *pthis) {
	memmanager *mman = memmanager_get();
	pthis->alive = 0;

	/* Libera el CPU alocado */
	free(pthis->pcpu);	
	
	/* Libera el segmento alocado */
	mman->liberar(pthis->mem);

	/* ME LIBERO A MI MISMA!!!!!!! */
	free(pthis);
}

int put_in_soup(celula *pcel) {
	if ((pcel->mem = malocar(pcel->size)))
		return 1;
	else 
		return 0;
}

void * vivir_thread(void *p) {
	int i;
	celula *pthis = (celula*)p;
	
	//envejece
	pthis->pvida--;
		
		/* ejecuto tantas instrucciones como me correspondan */
		for (i=0;i<pthis->slices;i++) {
			
			//EJECUTO!
			pthis->pcpu->run(pthis->pcpu);
			
			if (pthis->pcpu->fl == CPU_ERROR) {
				pthis->pvida--;
			}
			
			REC_instexec();
			
			/*
			pslicer->inst_exec++;
			
			if (pslicer->inst_exec%20000==0) {
				printf("inst. executed: %d\n", pslicer->inst_exec);	
			}
			*/
		}
		
#ifndef MT
		return NULL;
#endif
#ifdef MT
	//pthread_detach(pthread_self());
	pthread_exit(NULL);
#endif
}

static void lib (gpointer a, gpointer nada) {
	free(a);	
}

int load_from_bytes (celula *pthis, char *insts, int len) {
	memmanager *mman = memmanager_get();
	//int i;
	//printf ("New cel of size %d\n",len);
	//for (i=0;i<len;i++)
	//	printf("inst %d: %s\n",i,decodificar(*(insts+i)));
	pthis->size=len;
	while (!(pthis->mem = mman->malocar(pthis->size)));

	mman->load_cel(insts, pthis);
	return 1;	
}

int load_from_file(celula *pthis, char *filename) {
	char* coded_insts;
	GSList* idecoded_insts;
	memmanager *mman = memmanager_get();

	/* traigo una lista con las insts. sin encodear */	
	idecoded_insts = IO_fetch_insts(filename);
	
	pthis->size = genome_size(idecoded_insts);	
	
	if (!(pthis->mem = mman->malocar(pthis->size)))	
		return 0;
	
	// paso a binario mi lista de insts. 	
	coded_insts = codificar(idecoded_insts);		
	
	mman->load_cel(coded_insts, pthis);		
	
	// libero lo que use
	g_slist_foreach(idecoded_insts,lib,NULL);
	g_slist_free(idecoded_insts);	
	free(coded_insts);

	return 1;
}


int save(celula *pthis) {

#ifdef SAVE_GENOMAS
	if (pthis->size!=(pthis->padre)->size) return 1;
	
	char * padre_g = get_genoma(pthis->padre);
	char * hijo_g = get_genoma(pthis);

	/* solo guardo si el genoma se puede autoreplicar */
	if (are_clones(padre_g, hijo_g, pthis->size)) {
		if (GENOMA_time()) {
			if (!GB_newclone(hijo_g, pthis->size)) {
				free(hijo_g);
			}
		} else
			free(hijo_g);		
	} else
		free(hijo_g);

	free(padre_g);
#endif
	return 1;
}

int are_clones(char *padre_g, char *hijo_g, int size) {
	int i;
	
	if (padre_g==NULL) assert (0);	
	if (hijo_g==NULL) assert (0);	
	
	for (i=0;i<size;i++) {
			//printf("%s - %s\n",decodificar(*(padre_g+i)),decodificar(*(hijo_g+i)));
			if ((*(padre_g+i)^*(hijo_g+i))!=0) {
				//printf("dismatch %d: %x != %x\n",i, *(padre_g+i),*(hijo_g+i));
				return 0;
			}
	}
	
	return 1;
	
}

char* get_genoma (celula *pthis) {
	int i;
	//LIBERAR MEM, LIBERAR MEM!!!!!!!!!
	char *genoma = malloc(pthis->size);
	char c;

	for (i=0;i<pthis->size;i++) {
		c = get_byte((pthis->mem)->inicio+i);		
		*(genoma+i) = c;
		//memcpy((genoma+i),&c,1);	
	}
	return genoma;
}
