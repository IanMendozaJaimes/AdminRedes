/* librerías que usaremos */
//gcc conexion.c `mysql_config --cflags --libs`

#include "servidor_dhcp.h"


int main(){

	struct sockaddr_in addr, addr2;
	int servidor, leidos, servidor2, enviados;
	socklen_t addrlen, addrlen2;

	servidor = iniciar_servidor(GRUPO_BROADCAST, PUERTO_SERVIDOR, &addr);
	servidor2 = iniciar_servidor2(BROADCAST_INTERFAZ, PUERTO_CLIENTE, &addr2);
	addrlen = sizeof(addr);
	addrlen2 = sizeof(addr2);

	printf("El servidor esta listo y escuchando en el puerto %d.\n\n", PUERTO_SERVIDOR);

	while(1){

		struct respuesta res;
		struct conf_dhcp conf;
		unsigned char buffer[MAX_BUFFER];

		todo_nulo(&conf);

		if((leidos = recvfrom(servidor, buffer, MAX_BUFFER, 0,
			(struct sockaddr*) &addr, &addrlen)) < 0){
			error("No se pudo recibir el mensaje");
		}

		imprimir_trama(buffer, leidos);

		if(analizar_mensaje(&conf, buffer, leidos) < 0)
			continue;

		generar_cabecera_respuesta(&res, buffer, &conf);
		agregar_configuraciones(&res, &conf);

		printf("###########################");
		imprimir_trama(res.contenido, res.tam);

		if((enviados = sendto(servidor2, res.contenido, res.tam, 0,
			(struct sockaddr*) &addr2, addrlen2)) < 0){
			error("No se pudo enviar el mensaje");
		}

		printf("Se enviaron: %d bytes\n", enviados);
		cerrar_config(&conf);
		cerrar_res(&res);

	}

	return 0;
}


int analizar_mensaje(struct conf_dhcp * conf, unsigned char * msg, int tam){
	if(conf == NULL){return -1;}
	if(msg == NULL){return -1;}

	unsigned char * mac = (unsigned char *)malloc(6 * sizeof(char));
	conf -> ipv4 = (unsigned char *)calloc(4,  sizeof(char));

	// obtenemos la direccion fisica
	copiar_arreglo(mac, msg + 28, 6);
	printf("Direccion MAC: ");
	imprimir_trama(mac, 6);

	// obtenemos que tipo de mensaje es, es decir si es offer o request
	printf("%s\n", ".............................................");
	conf -> opciones = msg[242];
	if(conf -> opciones == 0x03){
		printf("%s\n", "Es un request");
		if(!obtener_decision_cliente(msg + 236)){
			printf("%s\n", "No quiso mi configuracion");
			eliminar_ip(mac);
			cerrar_config(conf);
			free(mac);
			return -1;
		}
		printf("%s\n", "Si quiso mi configuracion");
	}
	printf("%s\n", ".............................................");

	// obtenemos de la base de datos la configuracion de la compu con esa mac
	obtener_configuracion_guardada(mac, conf);
	registrar_ip(conf -> mac, conf -> ipv4);
	free(mac);

	return 1;
}

int obtener_decision_cliente(unsigned char * msg){
	if(msg == NULL){return 0;}

	int i;
	char opcion;

	for(i = 4;;){
		opcion = *(msg + i);
		if(opcion != 0x36){
			i += *(msg + i + 1) + 2;
			continue;
		}
		unsigned char ip_respuesta[4];
		unsigned char * ip_servidor;

		obtener_ip_servidor(&ip_servidor);
		copiar_arreglo(ip_respuesta, msg + i + 2, *(msg + i + 1));

		if(strcmp(ip_servidor, ip_respuesta) != 0){
			return 0;
		}
		break;
	}

	return 1;
}


int iniciar_servidor2(char * host, int puerto, struct sockaddr_in * addr){

	int sock;
	int broadcast = 1;

	if((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
		error("No se pudo crear el socket");
	}

	if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void*)&broadcast,
		 sizeof(broadcast)) < 0){
		error("No se pudieron dar permisos");
	}

	memset(addr, 0, sizeof(*addr));
	addr -> sin_family = AF_INET;
	addr -> sin_addr.s_addr = inet_addr(host);
	addr -> sin_port = htons(puerto);

	if(bind(sock, (struct sockaddr *)addr, sizeof(struct sockaddr)) == -1){
		error("No se pudo hacer el bind");
	}

	return sock;
}


int iniciar_servidor(char * host, int puerto, struct sockaddr_in * addr){
	struct ip_mreq mreq;
	int sock, si;

	// creamos el socket udp
	if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		error("No se pudo crear el socket");
	}


	// permitimos que multiples sockets utilicen el mismo puerto
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &si, sizeof(si)) < 0){
		error("Reuse address fallo");
	}

	// configuramos la direccion de destino
	memset(addr, 0, sizeof(*addr));
	addr -> sin_family = AF_INET;
	addr -> sin_addr.s_addr = htonl(INADDR_ANY);
	addr -> sin_port = htons(puerto);

	// hacemos el bind
	if(bind(sock, (struct sockaddr *) addr, sizeof(*addr)) < 0){
		error("No se pudo hacer el bind");
	}

	// nos unimos a un grupo multicast
	mreq.imr_multiaddr.s_addr = inet_addr(host);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	if(setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0){
		error("No se pudo unir al grupo multicast");
	}

	return sock;
}


int generar_cabecera_respuesta(struct respuesta * res, unsigned char * msg, struct conf_dhcp * conf){
	if(res == NULL){return -1;}
	if(conf == NULL){return -1;}

	int t = 241; // la cabecera basica mide 236, mas la magic cookie son otros 4, pero tambien contammos el caracter de fin de cadena
	res -> contenido = (unsigned char *)malloc(t*sizeof(char));
	res -> tam = t - 1;

	// op = 2, es respuesta
	res -> contenido[0] = 0x02;

	// ponemos el htype, hlen, hops, xid, secs, flas y clien ip address como en el msg
	copiar_arreglo(res -> contenido + 1, msg + 1, 15);

	// ponemos la yiaddr
	copiar_arreglo(res -> contenido + 16, conf -> ipv4, 4);

	// siaddr, giaddr, chaddr, padding del chaddr
	copiar_arreglo(res -> contenido + 20, msg + 20, 152);

	// unsigned char magic_cookie[4] = {63, 82, 53, 63};
	// ponemos la magic cookie
	res -> contenido[236] = 0x63;
	res -> contenido[237] = 0x82;
	res -> contenido[238] = 0x53;
	res -> contenido[239] = 0x63;

	res -> contenido[240] = '\0';

	return 0;
}


int agregar_configuraciones(struct respuesta * res, struct conf_dhcp * conf){
	if(res == NULL){return -1;}
	if(conf == NULL){return -1;}

	int i = 0;
	unsigned char * ip_servidor;
	unsigned char datos_fijos[2] = {0x02, 0x04};

	obtener_ip_servidor(&ip_servidor);

	// ponemos que es una respuesta
	if(conf -> opciones == 0x03)
		i = 1;
	agregar_configuracion(res, datos_fijos + i, 1, 0x35);

	// ponemos el identificador del servidor, osea su ip
	agregar_configuracion(res, ip_servidor, 4, 0x36);

	// ponemos el router, la puerta de enlace pues
	agregar_configuracion(res, conf -> enlace, 4, 0x03);

	// ponemos la mascara de red
	agregar_configuracion(res, conf -> mascara, 4, 0x01);

	// le pasamos el servidor dns
	agregar_configuracion(res, conf -> dns, 4, 0x06);

	res -> contenido[res -> tam] = 0xff;
	res -> tam += 1;

	return 0;
}


int agregar_configuracion(struct respuesta * res, unsigned char * datos,
				int tam, unsigned char tipo){
	if(res == NULL){return -1;}
	if(datos == NULL){return -2;}

	int nuevo_tam = 0;

	nuevo_tam = res -> tam + tam + 3;
	res -> contenido = realloc(res -> contenido, nuevo_tam);

	res -> contenido[res -> tam] = tipo;
	res -> contenido[res -> tam+1] = (unsigned char)(tam & 0x000000ff);

	copiar_arreglo(res -> contenido + res -> tam + 2, datos, tam);

	res -> contenido[nuevo_tam - 1] = '\0';
	res -> tam = nuevo_tam - 1;

	return 0;
}


int obtener_ip_servidor(unsigned char ** ip){
	if(ip == NULL){return -1;}

	*ip = (unsigned char *)malloc(4 * sizeof(char));
	(*ip)[0] = 0xc0; // 192
	(*ip)[1] = 0xa8; // 168
	(*ip)[2] = 0x01; // 1
	(*ip)[3] = 0x63; // 99

	return 0;
}


void copiar_arreglo(unsigned char * a, unsigned char * b, int tam){
	if(a == NULL){return;}
	if(b == NULL){return;}

	int i = 0;

	for(; i < tam; i++){
		a[i] = b[i];
	}
}

void cerrar_res(struct respuesta * res){
	if(res == NULL){return;}

	if(res -> contenido != NULL)
		free(res -> contenido);

}


void imprimir_trama(unsigned char * trama, int tam){
	printf("================================================\n");
	printf("La trama tiene un tam: %d bytes. \n\n", tam);
	int i = 0;
	for(;i < tam;){
		printf("%02x ", trama[i]);
		if(++i % 16 == 0){
			printf("\n");
		}
		else if(i % 8 == 0){
			printf(" ");
		}
	}
	printf("\n");
}
