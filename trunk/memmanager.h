#ifndef _MEMMANAGER_H
#define _MEMMANAGER_H

#include <glib.h>
#include "celula.h"

#define SOUP_SIZE 70000 //bytes
#define PROP_FREE 30 //porcentaje que el cleaner mantiene libre en la soup

typedef struct memmanager memmanager;
typedef struct segment segment;

struct memmanager {
	int total;
	int free;
	GSList *free_heap;
	GSList *used_heap;
	char *soup;	
	segment* (*malocar)(int csize);
	int (*load_cel)(char*, celula*);
	void (*init_heap)(void);
	void (*show_free_heap)(void);
	void (*show_used_heap)(void);
	void (*assert_mem)(void);
	int (*liberar)(segment *pmem);
	void (*defrag)(void);
	int (*enough_free)(void);
	int (*total_free)(void);
	void (*soup_mutate)(void);
};

struct segment {
	int id_cel;
	int inicio;
	int fin;
	int size;
};

segment* segment_new(int inicio, int size);
segment* segment_fit_search(int size);
void segment_resize(segment *pseg, int size);



char get_byte(int);
int memsize (void);
int set_byte(cpu*);
int liberar(segment *pmem);
void defrag (void);
void init_heap(void);
void assert_mem(void);
int total_free(void);
memmanager* memmanager_get(void);
void memmanager_reset(void);
segment* malocar(int csize);
int load_cel(char*, celula*);
void show_free_heap(void);
void show_used_heap(void);
int enough_free(void);
int auth_w(cpu*);
void soup_mutate(void);
int auth_divide(cpu *);
#endif
