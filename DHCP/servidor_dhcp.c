#include "servidor_dhcp.h"


int main(){
	
	struct sockaddr_in addr;
	struct respuesta res;
	struct mensaje_info msg_info;
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
		analizar_mensaje(&msg_info, buffer, leidos);
		generar_cabecera_respuesta(&res, buffer, &msg_info);		
		imprimir_trama(res.contenido, res.tam);		
	}

	return 0;
}


int analizar_mensaje(struct mensaje_info * info, unsigned char * msg, int tam){
	if(info == NULL){return -1;}
	if(msg == NULL){return -1;}
	
	// obtenemos el hardware address
	info -> haddr_len = msg[2];

	info -> haddr = (unsigned char *)malloc(info -> haddr_len * sizeof(char));
	info -> ip = (unsigned char *)calloc(4,  sizeof(char));
	info -> tid = (unsigned char *)malloc(4 * sizeof(char));

	// obtenemos el transaction id
	copiar_arreglo(info -> tid, msg + 4, 4);

	// obtenemos la direccion fisica
	copiar_arreglo(info -> haddr, msg + 28, info -> haddr_len);

	// ya con su direccion fisica, podemos ver si tenemos una ip asignada
	// a esta maquina, esto tiene que hacerce con tablas en una base de datos
	// que por cierto, aun no tenemos jeje, por eso lo vamos a hacer feo, y le
	// daremos una ip ya fija
	
	// ====================================
	info -> ip[0] = 0xc0;
	info -> ip[1] = 0xa8;
	info -> ip[2] = 0x01;
	info -> ip[3] = 0x64;
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

int generar_cabecera_respuesta(struct respuesta * res, unsigned char * msg, struct mensaje_info * info){
	if(res == NULL){return -1;}
	
	int t = 237; // la cabecera mide 236 pero tambien contammos el caracter de fin de cadena
	res -> contenido = (unsigned char *)malloc(t*sizeof(char));
	res -> tam = t - 1;

	// op = 2, es respuesta
	res -> contenido[0] = 0x02;
	
	// ponemos el htype, hlen, hops, xid, secs, flas y clien ip address como en el msg
	copiar_arreglo(res -> contenido + 1, msg + 1, 15);
	
	// yiaddr, vamos a hacerlo feo, dando una ip fija
	copiar_arreglo(res -> contenido + 16, info -> ip, 4);	
	
	// siaddr, giaddr, chaddr, padding del chaddr
	copiar_arreglo(res -> contenido + 20, msg + 20, 152);

	res -> contenido[236] = '\0';		

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
