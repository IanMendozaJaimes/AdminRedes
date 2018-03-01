#include "servidor_dhcp.h"


int main(){
	
	struct sockaddr_in addr;
	int servidor, leidos;
	socklen_t addrlen;

	servidor = iniciar_servidor(GRUPO_BROADCAST, PUERTO_CLIENTE, &addr);
	addrlen = sizeof(addr);
	
	printf("El servidor esta listo y escuchando en el puerto %d.\n\n", PUERTO_CLIENTE);
	
	while(1){

		unsigned char buffer[MAX_BUFFER];

		if((leidos = recvfrom(servidor, buffer, MAX_BUFFER, 0, (struct sockaddr*) &addr, &addrlen)) < 0){
			error("No se pudo recibir el mensaje");
		}

		imprimir_trama(buffer, leidos);
		
	}

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
