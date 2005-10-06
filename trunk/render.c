#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "estadistica.h"
#include "ui.h"
#include "globals.h"

static int clases_cont = 0;
static int errores_cont = 0;
static int total_mem = 0;
static char linea[100];
static float fact=1.0;
/*
static char *err_t[]={"SEG_FAULTS","MEM_FULL", "SEG_NULL", "ASSERT_SEG", "MEM_DESB", "MALOC_E", "THREAD"};
*/
static void each_clase (gpointer elem, gpointer nada) {
	struct clase_cel *pclase = (struct clase_cel*)elem;
	int i, max;
	int cant;
	int ancho=60;
	
	if (clases_cont > 19) return;

	if (((float)pclase->cantidad/ancho) > fact)
		fact = ((float)pclase->cantidad/ancho);
	
	max = (int)((float)pclase->cantidad/fact); 
	
	cant = (int)(((float)(ancho*pclase->cantidad*pclase->genome_size))/(float)total_mem);
	
	if (cant<0 && pclase->cantidad > 0) cant=1;	
	
	ui_drawi(0,8+clases_cont, pclase->genome_size);	
	for(i=0;i<ancho;i++) { 
		if (i < cant)
			linea[i]='*';
		else
			linea[i]=' ';
	}
	ui_drawi(5,8+clases_cont, pclase->cantidad);
	ui_draws(11,8+clases_cont, linea);
	clases_cont++;
}
/*
static void each_error (gpointer elem, gpointer nada) {
        struct clase_error *pclase = (struct clase_error*)elem;
                                                                                                 
        ui_draws(65,2+errores_cont, err_t[pclase->tipo]);
        ui_drawi(76,2+errores_cont, pclase->cantidad);
        errores_cont++;
}
*/
void render_init(void) {
	ui_init();	
}

void render_title(void) {
	ui_draws(0,0,"Running Arena...");
	ui_draws(20,2,"Generation Rate:  ");
	ui_drawi(46,2, INST_PER_GEN);
	ui_draws(0,2, "Insts execs: ");	
	ui_draws(0,3, "Mem size: ");	
	ui_draws(0,4, "Cels Vivas: ");	
	ui_draws(0,5, "Cels Creadas: ");	
	ui_draws(0,6, "Cels Eliminadas: ");	
}

void render_loop(void) {
	struct estadistica *est;
	est = estadistica_get();
	ui_drawi(15,2, est->instexec_total);	
	ui_draws(30,2, "Generacion: ");	
	ui_drawi(42,2, est->generacion);	
	ui_drawi(15,3, total_mem=est->memtotal);	
	ui_drawi(15,4, est->ncels);
	ui_drawi(15,5, est->inc_cels);
	ui_drawi(17,6, est->dec_cels);
	ui_draws(30,6, "Genoma salvado: ");	
	ui_draws(46,6, est->last_saved_genoma);
	errores_cont=0;
	clases_cont=0;
	fact=1.0;
	//g_slist_foreach(get_errores(), each_error, NULL);
	g_slist_foreach(get_clases(), each_clase, NULL);
}
