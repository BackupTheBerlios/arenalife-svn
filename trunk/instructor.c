#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "instructor.h"
#include "memmanager.h"

#define MAX_FILE_LEN 70

struct instructions insts[]= {
	{.idecoded="nop0", .icoded=0x00},
	{.idecoded="nop1", .icoded=0x01},
	{.idecoded="pushax", .icoded=0x02},
	{.idecoded="pushbx", .icoded=0x03},
	{.idecoded="pushcx", .icoded=0x04},
	{.idecoded="pushdx", .icoded=0x05},
	{.idecoded="popax", .icoded=0x06},
	{.idecoded="popbx", .icoded=0x07},
	{.idecoded="popcx", .icoded=0x08},
	{.idecoded="popdx", .icoded=0x09},
	{.idecoded="movcd", .icoded=0x0a},
	{.idecoded="movab", .icoded=0x0b},
	{.idecoded="movii", .icoded=0x0c},
	{.idecoded="sub_ab", .icoded=0x0d},
	{.idecoded="sub_ac", .icoded=0x0e},
	{.idecoded="inc_a", .icoded=0x0f},
	{.idecoded="inc_b", .icoded=0x10},
	{.idecoded="dec_c", .icoded=0x11},
	{.idecoded="inc_c", .icoded=0x12},
	{.idecoded="zero", .icoded=0x13},
	{.idecoded="not0", .icoded=0x14},
	{.idecoded="shl", .icoded=0x15},
	{.idecoded="ifz", .icoded=0x16},
	{.idecoded="jmp", .icoded=0x17},
	{.idecoded="jmpb", .icoded=0x18},
	{.idecoded="call", .icoded=0x19},
	{.idecoded="ret", .icoded=0x1a},
	{.idecoded="adr", .icoded=0x1b},
	{.idecoded="adrb", .icoded=0x1c},
	{.idecoded="adrf", .icoded=0x1d},
	{.idecoded="mal", .icoded=0x1e},
	{.idecoded="divide", .icoded=0x1f},
	{NULL, 0}
};

static char *dir;

/* guardo en la variable global dir el path relativo donde se guardaran los genomas */
void IO_define_path(char *d) {
	int len = strlen(d);
	int i, cont=0;
	char *cl[]={".","/"};	
	char *dirl = malloc(len+4); //. / \0 /
	memset(dirl,0,len+4);


	for (i=0;i<2;i++) {
		if (i<=len) {
			if (strncmp(d+i,cl[i],1)==0) {
				cont++;
			}
		}	
		strncpy(dirl+i,cl[i],1);
	}
	strncat(dirl,d+cont, len-cont);

	if (strncmp(dirl+strlen(dirl)-1,"/",1)!=0)
		strncat(dirl,"/",1);

	dir=dirl;
}

GSList *IO_fetch_insts(char *filename) {

	int i, ign, filefd;
	int noteof=1;
	char *inst=NULL;
	char car;
	GSList *insts = NULL;
		
	if ((filefd = open(filename, O_RDONLY)) == -1) { 
		perror("open");
		return NULL;
	} else {
		while (noteof) {
			inst = malloc(sizeof(MAX_INST_SIZE));
			i=0;
			ign=0;
			memset(inst,0,MAX_INST_SIZE);
			while ((noteof = read(filefd, &car,1)) > 0) {
				if (car == '\n') {
					*(inst+i) = 0;
					break;
				}
				
				/* ignoro comentarios y espacios */
				if (car == ';' || car == ' ' || ign==1) {
					ign=1;
					continue;
				}
				*(inst+i)=car;
				i++;
			}
			if (strlen(inst)<1) {
				free(inst);	
				break;
			}
			insts = g_slist_append(insts, inst);
		}
	}
	close(filefd);

	return insts;
}

static int c=0;
static void each_inst(gpointer data, gpointer user_data) {
	int i, found=0;
	char *inst = (char*)data;
	char *coded_insts = (char*)user_data;
	for (i=0;insts[i].idecoded!=NULL;i++) {
		if (!strcmp(inst, insts[i].idecoded)) {
			coded_insts[c]=insts[i].icoded;
			found=1;
			break;
		}
	}
	c++;
	if (found==0)
		printf("INSTRUCCION %s NO ENCONTRADA!\n", inst);
	
}

/* int to str */
static char *itos(int val) {
      char *str;
      int i, len = 0;
      if(val == 0) {
	      str = malloc(2);
	      *str = '0';
	      *(str+1) = '\0';
	      return(str);
      }

      i = val;

      for(i=val; i /= 10; len++);

      str = malloc(len + 1);
      str += len+1;

      do {
	      *--str = (val % 10) + '0';
	      //*(str++) = (val % 10) + '0';
	      //*(str++) = '0';
      } while(val /= 10);

      str[++len] = '\0';

      return(str);
}

pthread_mutex_t io;
char * IO_save_genoma(char *genoma, int size_i, char *ext) {
	int i;
	int filefd=-1;
	char *inst;
	
	// LIBERAR MEM! LIBERAR MEM!
	char *filename = malloc(MAX_FILE_LEN);
	char *size_s = itos(size_i);
	memset(filename, 0, MAX_FILE_LEN);

	if ((strlen(dir)+strlen(size_s))>MAX_FILE_LEN-4) return NULL; 
	
	//MUTEX LOCK
	pthread_mutex_lock(&io);
	
	strncat(filename,dir,strlen(dir));
	strncat(filename,size_s,strlen(size_s));
	strncat(filename,ext,strlen(ext));
	
	if ((filefd = open(filename, O_CREAT|O_WRONLY|O_EXCL|O_TRUNC, 0666)) == -1) {
		goto fin;
	} 
	

	for (i=0;i<size_i;i++) {
		inst = decodificar((int)*(genoma+i));
		write(filefd, inst, strlen(inst));
		write(filefd, "\n", 1);
	}
	//MUTEX UNLOCK
fin:
	pthread_mutex_unlock(&io);
	free(size_s);
	close(filefd);
	return filename;
}


char *codificar(GSList *idecoded_insts) {
	char *coded_insts = (char*)malloc(MAX_GENOME);
	memset(coded_insts, 0, MAX_INST_SIZE);
	
	g_slist_foreach(idecoded_insts, each_inst, coded_insts);
	c=0;

	return coded_insts;
}

/* decodifica 1 instruccion */
char *decodificar(char coded_inst) {
	int i;
	for (i=0;insts[i].idecoded!=NULL;i++) {
		if (!strncmp(&coded_inst, &insts[i].icoded, 1)) {
			return insts[i].idecoded;
		}
	}
	return "";
}

int genome_size(GSList *decoded_insts) {
	return g_slist_length(decoded_insts);
}
