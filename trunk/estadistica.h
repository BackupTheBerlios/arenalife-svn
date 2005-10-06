#ifndef _ESTADISTICA_H
#define _ESTADISTICA_H

#define SEG_FAULT 0
#define MEM_FULL 1
#define SEG_NULL 2
#define ASSERT_SEG 3
#define NEW_THREAD 4
#define MEM_DESBORD 5
#define MAL_ERROR 6

#include <glib.h>

typedef struct estadistica estadistica;
typedef struct clase_cel clase_cel;

struct estadistica {
	int inc_cels;
	int dec_cels;
	int ncels;
	int memfree;
	int memtotal;
	char last_saved_genoma[20];
	
	clase_cel *last_dom;	
	int last_dom_cont;
	
	/* Errores */
	int seg_faults;
	
	
	int generacion;
	unsigned long long int instexec;
	unsigned long long int instexec_total;
};

struct clase_cel {
	int genome_size;
	int cantidad;
};	

struct clase_error {
	int tipo;
	int cantidad;
};	


int get_cant_clase(int clase);
GSList* get_clases(void);
GSList* get_errores(void);
void REC_relist(void);
void REC_cel_creada(int);
void REC_cel_eliminada(int);
void REC_memory_free(int);
void REC_memory_total(int);
void REC_instexec(void);
void REC_reset_ronda(void);
void REC_reset(void);
void REC_error(int);
void REC_saved_genoma(char*);
struct estadistica* estadistica_get(void);

#endif
