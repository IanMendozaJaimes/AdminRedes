#include "servidor_dhcp.h"


int main(){
	
	struct sockaddr_in addr;
	struct respuesta res;
	struct conf_dhcp conf;
	int servidor, leidos;
	socklen_t addrlen;

	servidor = iniciar_servidor(GRUPO_BROADCAST, PUERTO_SERVIDOR, &addr);
	addrlen = sizeof(addr);
	
	printf("El servidor esta listo y escuchando en el puerto %d.\n\n", PUERTO_SERVIDOR);
	
	while(1){

		unsigned char buffer[MAX_BUFFER];

		if((leidos = recvfrom(servidor, buffer, MAX_BUFFER, 0, (struct sockaddr*) &addr, &addrlen)) < 0){
			error("No se pudo recibir el mensaje");
		}
		
		imprimir_trama(buffer, leidos);
		analizar_mensaje(&conf, buffer, leidos);
		generar_cabecera_respuesta(&res, buffer, &conf);
		agregar_configuraciones(&res, &conf);
		printf("###########################");	
		imprimir_trama(res.contenido, res.tam);		
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

	// ================================================================
	// ya con su direccion fisica, podemos ver si tenemos una configuracion
	// para la maquina que nos solicita, es decir, aqui se manda a llamar al
	// metodo de iniciar la estructura conf_dhcp, como no tenemos eso jeje,
	// le ponemos una ip asi fija
	// ====================================
	conf -> ipv4[0] = 0xc0;
	conf -> ipv4[1] = 0xa8;
	conf -> ipv4[2] = 0x01;
	conf -> ipv4[3] = 0x65;

	conf -> mac = mac;
	conf -> dns = NULL;
	conf -> mascara = NULL;
	conf -> enlace = NULL;
	// ====================================

	return 0;
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
	unsigned char datos_fijos[1] = {0x02};
	
	obtener_ip_servidor(&ip_servidor);
	
	// ponemos que es una respuesta 
	agregar_configuracion(res, datos_fijos, 1, 0x35);	
	
	// ponemos el identificador del servidor, osea su ip
	agregar_configuracion(res, ip_servidor, 4, 0x36);

	// ponemos el router, la puerta de enlace pues
	agregar_configuracion(res, conf -> enlace, 4, 0x03);

	// ponemos la mascara de red
	agregar_configuracion(res, conf -> mascara, 4, 0x01);

	// le pasamos el servidor dns
	agregar_configuracion(res, conf -> dns, 4, 0x0f);

	res -> contenido[res -> tam] = 0xff;

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
	
	//struct ifaddrs * ifaddr, * ifa;
	//int familia, s, n;

	*ip = (unsigned char *)malloc(4 * sizeof(char));
	(*ip)[0] = 0xc0; // 192
	(*ip)[1] = 0xa8; // 168
	(*ip)[2] = 0x01; // 1
	(*ip)[3] = 0x64; // 100

	//if(getifaddrs(&ifaddr) == -1){
	//	error("No se pudieron obtener las interfaces de red.");
	//}

	//for(ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa -> ifa_next, n++){
	//	if(ifa -> ifa_addr == NULL)
	//		continue;

	//	familia = ifa -> ifa_addr -> sa_family;
		
	//	if(strcmp(INTERFAZ_RED, ifa -> ifa_name) == 0){
	//		s = getnameinfo(ifa->ifa_addr, 
	//			(familia == AF_INET) ? sizeof(struct sockaddr_in):
	//					       sizeof(struct sockaddr_in6),
	//			*ip, 4, 
	//			NULL, 0, NI_NUMERICHOST);

	//		if(s != 0){
	//			error("No se pudo obtener la direccion ip del servidor");
	//		}

	//		break;
	//	}
		
	//}

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


void error(char * msg){
	perror(msg);
	exit(1);
}
