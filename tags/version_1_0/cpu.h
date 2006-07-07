#ifndef _CPU_H
#define _CPU_H

#define STACKSIZE 10
#define CPU_CLEAN 0
#define CPU_ERROR 1

typedef struct cpu cpu;

struct cpu{
	int ax; // address register
	int bx; // address register
	int cx; // numerical register
	int dx; // numerical register
	int fl; // flag
	int sp; // stack pointer
	int st[STACKSIZE]; // stack
	int ip; // instruction pointer	

	struct organismo *pcel;
	
	int instexec;

	int (*run)(cpu *);
};

int run (cpu *);
void show_regs(cpu *, int tiempo);

#endif
