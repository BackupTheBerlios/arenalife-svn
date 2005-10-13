#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include "arena.h"
#include "memmanager.h"
#include "estadistica.h"
#include "globals.h"
#include "render.h"
#include "mutation.h"
#include "netw.h"
#include "instructor.h"

void init_world(char *ancest, char *dir, int port) {
	
	//inicia el modulo de estadisticas
	REC_reset();

	//define donde se guardaran los genomas
	IO_define_path(dir);
	
	//inicia secuencia aleatoria
	random_init();
	
#ifdef UI
	//inicia la interfaz grafica
	render_init();

	//muestra titulos
	render_title();
#endif
	//inicia modulo networking y escucho puerto con otro hilo
	netw_init(port);
		
	//empieza...
	play_god(ancest);
}

void play_god(char *ancest) {
	
	int n,n2;
	estadistica *est = estadistica_get();
	memmanager *mman = memmanager_get();
	slicer *pslicer = slicer_get();
		
	//INCORPORA EL ANCESTRO
	if (ancest)
		pslicer->create_celula_from_file(ancest);
			
	
	/* Ronda para todas las celulas */
	while (pslicer->give_slice()) { 		

		pthread_mutex_lock(&pslicer->ronda_mutex);

		n=pslicer->ncels;
		n2=pslicer->get_total();
		assert(!(n2-n));
		assert(!(n2-est->ncels));
		
#ifdef DEBUG
		mman->show_free_heap();
		mman->show_used_heap();
		
		mman->assert_mem();
#endif

		
		if (est->instexec > REFRESH_RATE) {
#ifdef UI
			render_loop();	
#endif			
			REC_reset_ronda();
		}
	
		/* Mutacion al azar cada BG_MUTATION_RATE insts.*/	
		mman->soup_mutate();
		
		/* relista la lista del reaper para favorecer a los mas aptos */
		pslicer->relist();	
			
		/* libero memoria hasta llegar al umbral PROP_FREE */
		while (!mman->enough_free())
			pslicer->reaper();	
		
		
		/* antes de comenzar otra ronda, defragmento la RAM */
		mman->defrag();
		
		pthread_mutex_unlock(&pslicer->ronda_mutex);
	}
}
