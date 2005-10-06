#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include "genbank.h"
#include "celula.h"
#include "instructor.h"
#include "assert.h"
#include "globals.h"
#include "estadistica.h"

/* lista de genomas autoreplicables */
static GSList *genomas[MAX_CLASS];

static gint genoma_find(gconstpointer a, gconstpointer pthis) {
	clase_genoma *t = ((clase_genoma*)pthis);	
	clase_genoma *bankgen = ((clase_genoma*)a);
	if (t->size!=bankgen->size) return 1;
	if (are_clones(t->genoma,bankgen->genoma,t->size))
		return 0;
	else {
		return 1;
	}
}

static char * get_filename(gconstpointer c) {
	int i,j;
	GSList *clase = (GSList*)c;
	// LIBERAR! LIBERAR!
	char *ext=malloc(4);
	memset(ext,0,4);
	memset(ext,'a',3);
	for (i=0;i<g_slist_length(clase)-1;i++) {
		for(j=2;j>-1;j--) {
			if (*(ext+j)=='z') {
				if (j==0) goto fin;
				*(ext+j)='a';
				continue;
			}
			*(ext+j)+=1;
			break;
		}	
	}
	return ext;
fin:
	free(ext);	
	return NULL;
}

/* busca si ya se guardo este genoma, sino lo guarda al disco */
pthread_mutex_t newclone_t;
int GB_newclone(char *genoma, int size) {
	struct clase_genoma *newgeno = malloc(sizeof(*newgeno));
	char *ext = NULL;
	char *filename = NULL;
	newgeno->size = size;
	newgeno->genoma = genoma;
	pthread_mutex_lock(&newclone_t);
	if (!g_slist_find_custom(genomas[size], newgeno, genoma_find)) {
		//NUEVO GENOMA
		genomas[size] = g_slist_append(genomas[size], newgeno);	
		ext = get_filename(genomas[size]); 
		if (ext) {
			filename = IO_save_genoma(newgeno->genoma,newgeno->size, ext);
			if (filename) {
				REC_saved_genoma(filename);
				free(filename);
			}
			free(ext);
		}
		pthread_mutex_unlock(&newclone_t);	
		return 1;
	} else {
		free(newgeno);
		pthread_mutex_unlock(&newclone_t);	
		return 0;
	}
}
