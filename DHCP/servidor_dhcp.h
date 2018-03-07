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
	

	struct respuesta{
		unsigned char * contenido;
		int tam;
	};
	
	struct mensaje_info{
		char haddr_len;
		unsigned char * haddr; // hardware address, la mayoria del tiempo es la mac
		unsigned char * ip;
		unsigned char * tid; // transaction id
	};

	struct conf_dchp{
		char * mac;
		char * ipv4;
		char * mascara;
		char * enlace;
	};


	int iniciar_servidor(char*, int, struct sockaddr_in*);
	int generar_cabecera_respuesta(struct respuesta *, unsigned char *, struct mensaje_info *);
	int analizar_mensaje(struct mensaje_info *, unsigned char *, int);

	void imprimir_trama(unsigned char*, int);
	void error(char*);
	void copiar_arreglo(unsigned char *, unsigned char *, int);


#endif
