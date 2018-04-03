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
	#include <fcntl.h>
	#include <sys/stat.h>
	#include <unistd.h>

	#define PUERTO_SERVIDOR 67
	#define PUERTO_CLIENTE 68
	#define GRUPO_BROADCAST "225.255.255.255"
	#define BROADCAST_INTERFAZ "192.168.1.255"
	#define MAX_BUFFER 1024
	#define CAMPOS_DHCP 5
	#define INTERFAZ_RED "wlan0"


	struct respuesta{
		unsigned char * contenido;
		int tam;
	};

	struct conf_dhcp{
		unsigned char * mac;
		unsigned char * ipv4;
		unsigned char * mascara;
		unsigned char * enlace;
		unsigned char * dns;
		unsigned char opciones;
	};


	int iniciar_servidor(char *, int, struct sockaddr_in *);
	int iniciar_servidor2(char *, int, struct sockaddr_in *);
	int generar_cabecera_respuesta(struct respuesta *, unsigned char *, struct conf_dhcp *);
	int analizar_mensaje(struct conf_dhcp *, unsigned char *, int);
	int agregar_configuraciones(struct respuesta *, struct conf_dhcp *);
	int agregar_configuracion(struct respuesta *, unsigned char *, int, unsigned char);
	int obtener_ip_servidor(unsigned char **);
	int obtener_decision_cliente(unsigned char *);

	int conectar(MYSQL **);
	int obtener_configuracion_guardada(unsigned char *, struct conf_dhcp *);
	unsigned long convertir_char_entero(unsigned char *, int);
	int todo_nulo(struct conf_dhcp *);
	int llenar_informacion(struct conf_dhcp *, char *, char *, int, unsigned long);
	unsigned int obtener_ipv4(unsigned long);
	void imprimir_direcciones(unsigned char *, int, int);
	int registrar_ip(unsigned char *, unsigned char *);
	int eliminar_ip(unsigned char *);

	void imprimir_trama(unsigned char*, int);
	void error(char*);
	void copiar_arreglo(unsigned char *, unsigned char *, int);
	void cerrar_res(struct respuesta *);
	void cerrar_config(struct conf_dhcp *);


#endif
