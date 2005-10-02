#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <glib.h>
#include "instructor.h"

#define MAX_SIZE 4096

int main(int argc, char *argv[]) {
		
	int sockfd;
	int insts_len;
	struct sockaddr_in addr_remoto;
	socklen_t addr_remoto_len;
	char buffer[MAX_SIZE];
	int nread;
	unsigned short port;
	GList *GLinsts;
	char *insts;

	/* Verifico el nro de argumentos */
	if (argc != 4) {
		fprintf(stderr, "Uso: %s <nro ip> <puerto> <organismo>\n",
				argv[0]);
		return 255;
	}
	
	/* Codifico las instrucciones y las envio */
	GLinsts = IO_fetch_insts(argv[3]);
	if (!GLinsts) return 255;
	insts = codificar(GLinsts);
	insts_len = genome_size(GLinsts);

	/* Inicializo y establezco la estructura del cliente
	 * :1: familia (AF_INET)
	 * :2: IP
	 * :3: PORT
	 */
	memset(&addr_remoto, 0, sizeof(addr_remoto));
	
	/* :1: familia */
	addr_remoto.sin_family = AF_INET;
	
	/* :2: IP 
	 *     convierto la dir. IP de string a sin_addr
	 */
	if ( !(inet_aton(argv[1], &addr_remoto.sin_addr))) {
		perror("inet_aton");
		return 255;
	}
	/* 
	 * :3: PORT
	 *     convierto el port a numerico y luego la paso de 
	 *     representacion de CPU -> red 
	 */
	port = atoi(argv[2]);
	addr_remoto.sin_port = htons(port);
	addr_remoto_len = sizeof(addr_remoto);
	
	/* Creo el socket TCP */	
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0))<0) {
		perror("socket");
		return 3;
	}

	/* Conecta al peer remoto */
	if (connect(sockfd, (struct sockaddr *)&addr_remoto, addr_remoto_len)) {
		perror("connect");
		return 4;
	}
	
	
	if (write(sockfd, insts, insts_len)<0) {
		perror("write"); return 1;	
	}
	
	free(insts);


	/* no me interesa seguir escribiendo ...*/
	shutdown(sockfd, SHUT_WR);

	/* leo todo lo que haya escrito el peer y lo copio a stdout */
	while( (nread=read(sockfd, buffer, sizeof buffer)) > 0) {
		write(STDOUT_FILENO, buffer, nread);
	}
	
	close(sockfd);
	return EXIT_SUCCESS;
}
