#include <stdlib.h>
#include <time.h>
#include "mutation.h"
#include "globals.h"

void random_init(void) {
	srand(time(NULL));
}

int my_rand(int liminf, int limsup) {
	return liminf +(int) (((float)(limsup-liminf+1))*rand()/(RAND_MAX+1.0));
}

static int falla(int rate) {
	if (rand()>(RAND_MAX - RAND_MAX/rate))
		return 1;
	else
		return 0;
}

int COPY_falla(void) {
	return falla(COPY_MUTATION_RATE);
}

int CPU_falla(void) {
	return falla(CPU_MUTATION_RATE);
}

int SOUP_falla(void) {
	return falla(BG_MUTATION_RATE);
}

int GENOMA_time(void) {
	return falla(GENOMA_SAVE_RATE);
}

