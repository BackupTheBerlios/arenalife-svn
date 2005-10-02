#ifndef _MEMMANAGER_H
#define _MEMMANAGER_H

#include <glib.h>
#include "celula.h"

typedef struct memmanager memmanager;
typedef struct segment segment;

struct memmanager {
	int total;
	int free;
	GList *free_heap;
	GList *used_heap;
	char *soup;	
	int (*malocar)(celula*);
	int (*load_cel)(char*, celula*);
	void (*init_heap)(void);
	void (*show_free_heap)(void);
	void (*show_used_heap)(void);
	void (*assert_mem)(void);
	int (*liberar)(celula *pcel);
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

char get_byte(int);
int memsize (void);
int set_byte(cpu*);
int liberar(celula *pcel);
void defrag (void);
void init_heap(void);
void assert_mem(void);
int total_free(void);
memmanager* memmanager_get(void);
int malocar(celula*);
int load_cel(char*, celula*);
void show_free_heap(void);
void show_used_heap(void);
int enough_free(void);
int auth_w(cpu*);
void soup_mutate(void);
int auth_divide(cpu *);
#endif
