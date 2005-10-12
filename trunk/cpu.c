#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "cpu.h"
#include "slicer.h"
#include "instructor.h"
#include "memmanager.h"
#include "celula.h"
#include "mutation.h"
#include "globals.h"

#define FORWARD 1
#define BACKWARD 0

static int find_template(cpu *pcpu, int sent);
static int sizeof_template (cpu *pcpu);

/*void show_regs(cpu *pcpu, int tiempo) {
	printf("ax=%d\n",pcpu->ax);
	printf("bx=%d\n",pcpu->bx);
	printf("cx=%d\n",pcpu->cx);
	printf("dx=%d\n",pcpu->dx);
	printf("IP: %s\n", decodificar(get_byte(pcpu->ip)));
	sleep(tiempo);
}*/

//done
static void nop0(cpu *pcpu) {
	pcpu->ip++;
}	

//done
static void nop1(cpu *pcpu) {
	pcpu->ip++;
}	

/* flip del ultimo bit de CX */
//done
static void not0(cpu *pcpu) {
	if (CPU_falla())
		pcpu->cx ^= my_rand(0,2);
	else
		pcpu->cx ^= 1;
	pcpu->ip++;
}

//done
static void shl(cpu *pcpu) {
	if (CPU_falla())
		pcpu->cx <<= my_rand(0,2);	
	else 
		pcpu->cx <<=1;	
	pcpu->ip++;
}	

//done
static void ifz(cpu *pcpu) {
	if (pcpu->cx == 0)
		pcpu->ip++;
	else
		pcpu->ip = pcpu->ip + 2;
}

//done
static void sub_ab(cpu *pcpu) {
	pcpu->cx = pcpu->ax - pcpu->bx;
	if (CPU_falla())
		pcpu->cx-=my_rand(-1,1);
	pcpu->ip++;
}	

//done
static void sub_ac(cpu *pcpu) {
	pcpu->ax = pcpu->ax - pcpu->cx;
	if (CPU_falla())
		pcpu->ax-=my_rand(-1,1);
	pcpu->ip++;
}	

//done
static void inc_a(cpu *pcpu) {
	pcpu->ax++;
	if (CPU_falla())
		pcpu->ax-=my_rand(-1,1);
	pcpu->ip++;
}	

//done
static void inc_b(cpu *pcpu) {
	pcpu->bx++;
	if (CPU_falla())
		pcpu->bx-=my_rand(-1,1);
	pcpu->ip++;
}

//done
static void inc_c(cpu *pcpu) {
	pcpu->cx++;
	if (CPU_falla())
		pcpu->cx-=my_rand(-1,1);
	pcpu->ip++;
}

//done
static void dec_c(cpu *pcpu) {
	pcpu->cx--;
	if (CPU_falla())
		pcpu->cx+=my_rand(-1,1);
	pcpu->ip++;
}	
/*
static void show_pila(cpu *pcpu) {
	int i;
	for (i=0;i<STACKSIZE;i++) {
		if (pcpu->st[i]!=0)
			printf("pila[%d]=%d\n",i,pcpu->st[i]);
	}
}
*/
static int pila_full(cpu *pcpu) {
	if (pcpu->sp >= STACKSIZE-1) {
		pcpu->fl = CPU_ERROR;
		return 1;
	} else {
		return 0;	
	}
}

static int pila_empty(cpu *pcpu) {
	if (pcpu->sp <= 0) {
		pcpu->fl = CPU_ERROR;
		return 1;
	}
	else {
		return 0;
	}
}

static int pila_pop(cpu *pcpu) {
	if (pila_empty(pcpu)) return -1;
	else { 
		int temp = pcpu->st[--pcpu->sp];
		pcpu->st[pcpu->sp] = 0;
		return temp;	
	}
}

static void pila_push(cpu *pcpu, int valor) {
	if (pila_full(pcpu)) return;
	else
		pcpu->st[pcpu->sp++] = valor;
}

//done
static void pushax(cpu *pcpu) {
	pila_push(pcpu, pcpu->ax);	
	pcpu->ip++;
}

//done
static void pushbx(cpu *pcpu) {
	pila_push(pcpu, pcpu->bx);	
	pcpu->ip++;
}

//done
static void pushcx(cpu *pcpu) {
	pila_push(pcpu, pcpu->cx);	
	pcpu->ip++;
}	

//done
static void pushdx(cpu *pcpu) {
	pila_push(pcpu, pcpu->dx);	
	pcpu->ip++;
}

//done
static void popax(cpu *pcpu) {
	pcpu->ax = pila_pop(pcpu);
	pcpu->ip++;
}	

//done
static void popbx(cpu *pcpu) {
	pcpu->bx = pila_pop(pcpu);
	pcpu->ip++;
}	

//done
static void popcx(cpu *pcpu) {
	pcpu->cx = pila_pop(pcpu);
	pcpu->ip++;
}

//done
static void popdx(cpu *pcpu) {
	pcpu->dx = pila_pop(pcpu);
	pcpu->ip++;
}

//done
static void jmp(cpu *pcpu) {
	int temp;
	if ((sizeof_template(pcpu)==0)) {
		pcpu->ip = pcpu->ax;
	} else if ((temp = find_template(pcpu, BACKWARD))>-1) {
		pcpu->ip = temp;
	} else if ((temp = find_template(pcpu, FORWARD))>-1) {
		pcpu->ip = temp;
	} else {
		pcpu->fl = CPU_ERROR;
		pcpu->ip++;// = pcpu->ax;
	}
}

//done
static void jmpb(cpu *pcpu) {
	int temp;
	if ((sizeof_template(pcpu)==0)) {
		pcpu->ip = pcpu->ax;
	} else if ((temp = find_template(pcpu, BACKWARD))>-1) {
		pcpu->ip = temp;
	} else {
		pcpu->fl = CPU_ERROR;
		pcpu->ip++;// = pcpu->ax;
	}
}	

/* pone el IP en el stack y salta a la template complementaria */
//done
static void call(cpu *pcpu) {
	int temp;
	if (!pila_full(pcpu)) {
		pila_push(pcpu, (int)(pcpu->ip+1));
	}
	if ((temp = find_template(pcpu, FORWARD))>-1) {
		pcpu->ip = temp;
	} else {
		pcpu->fl = CPU_ERROR;
		pcpu->ip++;
	}
}	

//done
static void ret(cpu *pcpu) {
	if (!pila_empty(pcpu)) {
		pcpu->ip = pila_pop(pcpu);
	}
}

//done
static void movcd(cpu *pcpu) {
	pcpu->dx = pcpu->cx;
	pcpu->ip++;
}

//done
static void movab(cpu *pcpu) {
	pcpu->bx = pcpu->ax;
	pcpu->ip++;

}

//done
static void movii(cpu *pcpu) {
	if (set_byte(pcpu)<0) {	
		pcpu->fl = CPU_ERROR;
		//printf("byte %x\n",get_byte(pcpu->bx));
	}	
	pcpu->ip++;		
}	

static int find_template(cpu *pcpu, int sent) {
	int cont=0;
	int done=0;
	int founds=0, i;
	int max=SEARCH_LIMIT;
	int templsize=MAX_TEMPL_SIZE;
	char templ[templsize];
	int ip;
	int a=0;
	int ret=-1;
	memset(templ,0,templsize);

	int ip_backup = pcpu->ip;

	pcpu->ip++;
	char byte  = get_byte(pcpu->ip);
		
	while (byte==0X00 || byte==0X01) {
		if (cont<templsize) {
			templ[cont]=byte;
			templ[cont]^= 1; //flip
		}
		cont++;
		pcpu->ip++;
		if (get_byte(pcpu->ip)==-1)
			break;
		byte = get_byte(pcpu->ip);
		//if (!auth_r(ip))
		//	return 0;
		if (cont>max) break;
	}
	//printf("\n\n");
	while(!done) {
		a++;
		if (a>max) break;
		if (cont==0) break;
		if (cont>max) break;
		(sent==BACKWARD)?pcpu->ip--:pcpu->ip++;
		
		for (i=0;i<cont;i++) {
			//if (!auth_r(ip))
			//	return 0;
			pcpu->ip++;
			if (get_byte(pcpu->ip)==templ[i]) {
				if (get_byte(pcpu->ip)==-1) break;
				founds++;
			}
		}
		if (founds==cont) { 
			ret = 1;
			break;
		}
		pcpu->ip = pcpu->ip-cont;
		founds=0;
	}
	ip = pcpu->ip+1; //mas 1 para que sea la proxima inst. despues de la template
	pcpu->ip = ip_backup;
	if (ret==1)
		return ip;
	else
		return -1;
}

static int sizeof_template (cpu *pcpu) {
	int cont=0;
	int ip_backup = pcpu->ip;
	pcpu->ip++;
	char byte  = get_byte(pcpu->ip);
		
	while (byte==0X00 || byte==0X01) {
		if (cont<7) {
			cont++;
		} else
			break;
		pcpu->ip++;
		if ((byte=get_byte(pcpu->ip))==-1)
			break;
	}
	pcpu->ip = ip_backup;
	return cont;	
}

static void adr(cpu *pcpu) {
	int temp;
	if ((temp = find_template(pcpu, FORWARD))>-1) {
		pcpu->ax = temp;
		pcpu->cx = sizeof_template(pcpu);
		pcpu->ip+=pcpu->cx;
	}
	else if ((temp = find_template(pcpu, BACKWARD))>-1) {
		pcpu->ax = temp;
		pcpu->cx = sizeof_template(pcpu);
		pcpu->ip+=pcpu->cx;
	}
	pcpu->ip++;
}

//done
static void adrb(cpu *pcpu) {
	int temp;
	if ((temp = find_template(pcpu, BACKWARD))>-1) {
		pcpu->ax = temp;
		pcpu->cx = sizeof_template(pcpu);
		pcpu->ip+=pcpu->cx;
	}	
	pcpu->ip++;
}		

//done
static void adrf(cpu *pcpu) {
	int temp;
	if ((temp = find_template(pcpu, FORWARD))>-1) {
		pcpu->ax = temp;
		pcpu->cx = sizeof_template(pcpu);
		pcpu->ip+=pcpu->cx;
	}// else
		//printf("Template not found\n");
	pcpu->ip++;
}

//done
pthread_mutex_t maloc;
static void mal(cpu *pcpu) {
	celula *phijo = pcpu->pcel->hijo;
	
	pcpu->ip++;

	//if (pcpu->cx!=45 && pcpu->cx!=80) return;
	
	//TEMPORAL: solo 2 hijos por ronda
	if ((pcpu->pcel)->ronda_hijos>2) return;
	
	//si ya tuve un hijo y este es independiente ó si no tuve hijos
	if (phijo==NULL) {
		if (pcpu->cx > 19 && pcpu->cx < MAX_CLASS) {
			celula *pcel = celula_new();
			phijo = pcel;	
	
			phijo->size = pcpu->cx;
			//printf("%d\n",phijo->size);
			//pthread_mutex_lock(&maloc);
			if ((phijo->mem = Vmalloc(phijo->size))) {
				pcpu->ax = (phijo->mem)->inicio;
				phijo->pcpu->ip = pcpu->ax;
				(pcpu->pcel)->hijo = phijo;
			} else
				pcel->die(pcel);
			//pthread_mutex_unlock(&maloc);
		}
	}
}

//done
pthread_mutex_t divide_f;
static void divide(cpu *pcpu) {
	celula *phijo;

	if (!auth_divide(pcpu)) {
		pcpu->ip++;
		return;
	}

	//si tengo algun hijo (maloc bien)...
	pthread_mutex_lock(&divide_f);
	if ((pcpu->pcel)->hijo) {
		if ((phijo = (pcpu->pcel)->hijo)) {
			if ((pcpu->pcel)->id > -1) {
				//lista_put(cola, phijo);
				phijo->padre = (pcpu->pcel);
				
				crear_hijo(phijo);
				
				//premio
				//solo si es clone
				//(pcpu->pcel)->pvida++;
				
				//Ya es independiente
				(pcpu->pcel)->hijo = NULL;
				(pcpu->pcel)->ronda_hijos++;
			}
		}
	}
	pthread_mutex_unlock(&divide_f);
	pcpu->ip++;
	
}	

//done
static void zero(cpu *pcpu) {
	pcpu->cx = 0;
	pcpu->ip++;
}

int run(cpu *pcpu) {
	
	char byte;
	
	pcpu->fl = CPU_CLEAN;

	if ((byte = get_byte(pcpu->ip)) < 0) {
		pcpu->fl = CPU_ERROR;
		pcpu->ip++;	
		return -1;
	}
	//printf("Cel id-> %d, byte : 0x%x, at IP: %s\n",pcpu->pcel->id ,byte, decodificar(get_byte(pcpu->ip)));	
	//show_regs(pcpu,0);	
	switch(byte) {
      	case 0X00   : nop0(pcpu);   break;
      	case 0X01   : nop1(pcpu);   break;
                                                                                                    
      	case 0X02 : pushax(pcpu); break;
      	case 0X03 : pushbx(pcpu); break;
      	case 0X04 : pushcx(pcpu); break;
      	case 0X05 : pushdx(pcpu); break;
      	case 0X06 : popax(pcpu);  break;
      	case 0X07 : popbx(pcpu);  break;
      	case 0X08 : popcx(pcpu);  break;
      	case 0X09 : popdx(pcpu);  break;
      	case 0X0a : movcd(pcpu);  break;
      	case 0X0b : movab(pcpu);  break;
      	case 0X0c : movii(pcpu); break;
                                                                                                    
      	case 0X0d : sub_ab(pcpu);  break;
      	case 0X0e : sub_ac(pcpu);  break;
      	case 0X0f : inc_a(pcpu);   break;
      	case 0X10 : inc_b(pcpu);   break;
      	case 0X11 : dec_c(pcpu);   break;
      	case 0X12 : inc_c(pcpu);   break;
      	case 0X13 : zero(pcpu);    break;
      	case 0X14 : not0(pcpu);    break;
      	case 0X15 : shl(pcpu);     break;
      
      	case 0X16 : ifz(pcpu);   break;
      	case 0X17 : jmp(pcpu);     break;
      	case 0X18 : jmpb(pcpu);   break;
      	case 0X19 : call(pcpu);    break;
      	case 0X1a : ret(pcpu);     break;
                                                                                                    
      	case 0X1b : adr(pcpu);     break;
	case 0X1c : adrb(pcpu);   break;
	case 0x1d : adrf(pcpu);   break;
      	case 0X1e : mal(pcpu);     break;
      	case 0X1f : divide(pcpu);  break;
	//default:printf("CPU: Instruccion No Encontrada!\n");break;	
	}
	return 0;
}
