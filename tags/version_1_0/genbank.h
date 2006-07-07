#ifndef _GENBANK_
#define _GENBANK_

#include "organismo.h"

typedef struct clase_genoma clase_genoma;

struct clase_genoma {
	int size;
	char *genoma;
	char *sug_name;
};

int GB_newclone(char *, int);

#endif
