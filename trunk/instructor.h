#ifndef _INSTRUCTOR_H
#define _INSTRUCTOR_H

#define MAX_INST_SIZE 10
#define MAX_GENOME 2000

#include <glib.h>

struct instructions {
	char *idecoded;
	char icoded;
};

void IO_define_path(char *);
GSList *IO_fetch_insts(char *);
char *codificar(GSList *);
char *decodificar(char);
int genome_size(GSList *);
char * IO_save_genoma(char *, int, char *);

#endif
