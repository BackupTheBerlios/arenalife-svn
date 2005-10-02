#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include "slicer.h"

static void * listen_thread (void *);

/* Crea un thread que escuchara el puerto esperando alguna inyeccion */
void netw_init(unsigned int port) {
	pthread_t tid;
	int *pport = malloc(sizeof(int));
	*pport = port;
	if (pthread_create(&tid, NULL, listen_thread, (void*)pport)!=0) {	
		perror ("pthread_create()"); return;
	}
}

char *resp = "Respuesta desde Arena: Celula inyectada con exito!\n";

static void * inject (void *fd) {
	int sockfd = *((int*)fd);
	int bytes;
	int maxlen = 1000;
	char *genoma = malloc(maxlen);
	memset (genoma,0, maxlen);
	slicer *pslicer = slicer_get();	
	
	bytes = read (sockfd, genoma, maxlen);	
	
	if (bytes <= 0) {
		perror("read()");
		return NULL;
	}
		
	if ((write(sockfd, resp,strlen(resp)) <= 0)) {
		perror("write()");
		return NULL;
	}
		
	/* al slicer ... */
	pslicer->create_celula_from_bytes(genoma, bytes);
	
	free(genoma);

	//despierto al slicer en caso ...
	pthread_mutex_lock(&pslicer->wait_m);
	pthread_cond_signal(&pslicer->wait_c);
	pthread_mutex_unlock(&pslicer->wait_m);

	close(sockfd);
	pthread_detach(pthread_self());
	pthread_exit(NULL);
}

static void * listen_thread (void *p) {

	int port = *((int*)p);;
	int sockfd, servfd, opt;
        struct sockaddr_in addr_in;
	pthread_t tid;
        
	if(( sockfd=socket(PF_INET, SOCK_STREAM, 0))<0) {
                perror("socket()");assert(0);
        }

	memset(&addr_in, 0, sizeof(struct sockaddr_in));
        addr_in.sin_family = AF_INET;
        addr_in.sin_port = htons(port);
        addr_in.sin_addr.s_addr = INADDR_ANY;

	opt = 1;
	
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt))<0) {
                perror("setsockopt()");assert(0);
        }
	
	if (bind (sockfd, (struct sockaddr *)&addr_in, sizeof(struct sockaddr_in))<0) {
               perror("bind()");assert(0);
	}

	if ((listen(sockfd, 20))<0) {
                perror("listen()");assert(0);
	}
        
	while ((servfd=accept(sockfd, NULL, 0))>0) {	
		/* otro thread se encargara de la inyeccion */
		if (pthread_create(&tid, NULL, inject, (void*)&servfd)!=0) {
			perror ("pthread_create()"); return NULL;
		}
	}
	printf("fin listening...\n");
	pthread_detach(pthread_self());
	pthread_exit(NULL);
}
