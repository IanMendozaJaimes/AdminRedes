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
	#include <ifaddrs.h>
	#include <netdb.h>	
	#include <my_global.h>
	#include <mysql.h>

	#define PUERTO_SERVIDOR 67
	#define PUERTO_CLIENTE 68
	#define GRUPO_BROADCAST "225.255.255.255"
	#define MAX_BUFFER 1024
	#define CAMPOS_DHCP 5
	#define INTERFAZ_RED "wlan0"
	

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

	struct conf_dhcp{
		char * mac;
		char * ipv4;
		char * mascara;
		char * enlace;
		char * dns;
	};


	int iniciar_servidor(char*, int, struct sockaddr_in*);
	int generar_cabecera_respuesta(struct respuesta *, unsigned char *, struct conf_dhcp *);
	int analizar_mensaje(struct conf_dhcp *, unsigned char *, int);
	int agregar_configuraciones(struct respuesta *, struct conf_dhcp *);	
	int agregar_configuracion(struct respuesta *, unsigned char *, int, unsigned char);
	int obtener_ip_servidor(unsigned char **);
	int conseguir_configuracion(struct conf_dhcp *config,unsigned char *mac);

	void imprimir_trama(unsigned char*, int);
	void error(char*);
	void copiar_arreglo(unsigned char *, unsigned char *, int);


#endif
