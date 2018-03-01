#ifndef __SERVIDOR_DHCP__
#define __SERVIDOR_DHCP__

	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <time.h>

	#define PUERTO_SERVIDOR 67
	#define PUERTO_CLIENTE 68
	#define GRUPO_BROADCAST "225.255.255.255"
	#define MAX_BUFFER 1024


	int iniciar_servidor(char*, int, struct sockaddr_in*);
	void imprimir_trama(char*, int);
	void error(char*);



#endif
