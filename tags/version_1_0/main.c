#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "arena.h"

extern char *optarg;
extern int optind, opterror, optopt;

char *opc_ancest = NULL;
char *opc_dir = NULL;
int opc_port;
const char *progname;

void usage(FILE *f, int exit_status) {
	fprintf(f, "ERROR: uso %s -a ancest|'net' -d dir -p port\n", progname);
	exit(exit_status);
}

int main(int argc, char* argv[]) {
	int c, filefd;
	DIR *pdir;
	progname=argv[0];
        while ((c=getopt(argc, argv, "d:a:p:"))>=0) {
		switch (c) {
			case 'd':
				opc_dir = optarg;
				break;
			case 'p':
				opc_port = atoi(optarg);
				break;
			case 'a':
				opc_ancest = optarg;
				break;
			default:
				usage(stderr, 255);
				break;
		}
	}
	if (argc!=7) usage(stderr, 255);
        
	argc-=(optind-1);
        argv+=(optind-1);
	
	if (opc_port < 0) {
		fprintf(stderr, "Puerto %d incorrecto.\n", opc_port);
		exit(255);
	}
	
        if ((pdir = opendir(opc_dir)) == NULL) {
		fprintf(stderr, "No se puede leer el directorio: %s\n", opc_dir);
		exit(255);
	}
	closedir(pdir);
	if(strncmp(opc_ancest,"net",3)==0) {
		opc_ancest = NULL;
	} else if ((filefd = open(opc_ancest, O_RDONLY)) == -1) {
		fprintf(stderr, "No se puede leer el archivo ancestro: %s\n",opc_ancest);
		exit(255);	
	}
	close(filefd);
	//printf("ancest: %s\n",opc_ancest);
	//printf("dir: %s\n",opc_dir);
	
	init_world(opc_ancest, opc_dir, opc_port);
	
	return 0;
}
