#include "servidor_dhcp.h"

unsigned long convertir_char_entero(unsigned char * mac, int tam){
	if(mac == NULL){return 0;}

	unsigned long nueva_mac = 0;
	unsigned long aux = 0;
	int i = 0;

	for(; i < tam; i++){
		aux = mac[tam-1-i];
		aux <<= i*8;
		nueva_mac += aux;
	}

	return nueva_mac;
}

int convertir_entero_a_char(unsigned char ** arreglo, unsigned int dato){
		if(arreglo == NULL){return -1;}

		int i;

		for(i = 0; i < 4; i++){
			(*arreglo)[i] = (dato >> (8*(3-i))) & 0xff;
		}

		return 0;
}

int obtener_configuracion_guardada(unsigned char * mac, struct conf_dhcp * conf){
	if(mac == NULL){return -1;}

	MYSQL * con;
	conectar(&con);

	unsigned long nueva_mac = convertir_char_entero(mac, 6);
	conf -> mac = (unsigned char *)malloc(6 * sizeof(char));
	char query[500];
	int i = 0;

	(conf -> mac)[0] = mac[0];
	(conf -> mac)[1] = mac[1];
	(conf -> mac)[2] = mac[2];
	(conf -> mac)[3] = mac[3];
	(conf -> mac)[4] = mac[4];
	(conf -> mac)[5] = mac[5];

	sprintf(query, "call spObtenerConfiguracion(%lu);", nueva_mac);

	if(mysql_query(con, query)){
		error("No se pudo obtener la informacion de la base de datos.");
	}

	MYSQL_RES * resultset = mysql_store_result(con);
	MYSQL_ROW row;

	if(resultset == NULL){
		todo_nulo(conf);
		return 0;
	}

	if((row = mysql_fetch_row(resultset))){
		for(; i < 4; i++)
			llenar_informacion(conf, row[i], row[6-i], i, nueva_mac);
	}

	mysql_free_result(resultset);
	mysql_close(con);

	return 0;
}

int llenar_informacion(struct conf_dhcp * conf, char * permiso,
	char * dato, int tipo, unsigned long mac){
	if(permiso == NULL){return -1;}
	if(dato == NULL){return -1;}
	if(conf == NULL){return -1;}

	unsigned int dato_aux;
	int permiso_aux;

	dato_aux = atoi(dato);
	permiso_aux = atoi(permiso);

	if(tipo == 0){ // dns
		if(permiso_aux == 1){
			conf -> dns = (unsigned char *)malloc(4 * sizeof(char));
			convertir_entero_a_char(&(conf -> dns), dato_aux);
		}
		else{
			conf -> dns = NULL;
		}
	}
	else if(tipo == 1){ // enlace
		if(permiso_aux == 1){
			conf -> enlace = (unsigned char *)malloc(4 * sizeof(char));
			convertir_entero_a_char(&(conf -> enlace), dato_aux);
		}
		else{
			conf -> enlace = NULL;
		}
	}
	else if(tipo == 2){ // mascara
		if(permiso_aux == 1){
			conf -> mascara = (unsigned char *)malloc(4 * sizeof(char));
			convertir_entero_a_char(&(conf -> mascara), dato_aux);
		}
		else{
			conf -> mascara = NULL;
		}
	}
	else{
		if(permiso_aux == 1){
			dato_aux = obtener_ipv4(mac);
			conf -> ipv4 = (unsigned char *)malloc(4 * sizeof(char));
			convertir_entero_a_char(&(conf -> ipv4), dato_aux);
		}
		else{
			conf -> ipv4 = NULL;
		}
	}

	return 0;
}

unsigned int obtener_ipv4(unsigned long mac){
	char arreglo[200];
	unsigned int ip = 0;
	MYSQL * con;

	conectar(&con);
	sprintf(arreglo, "call spObtenerIpv4(%lu);", mac);

	if(mysql_query(con, arreglo)){
		printf("%s\n", arreglo);
		error("No se pudo obtener la ipv4.");
	}

	MYSQL_RES * resultset = mysql_store_result(con);
	MYSQL_ROW row;

	if(resultset == NULL){
		error("No se pudo obtener la ipv4 prro.");
	}

	if((row = mysql_fetch_row(resultset))){
		ip = atoi(row[0]);
	}

	mysql_free_result(resultset);
	mysql_close(con);

	return ip;
}

int registrar_ip(unsigned char * mac, unsigned char * ipv4){
	if(mac == NULL){return -1;}
	if(ipv4 == NULL){return -1;}

	MYSQL * con;
	char query[150];
	unsigned long nueva_mac = convertir_char_entero(mac, 6);
	unsigned int nueva_ip = (unsigned int)convertir_char_entero(ipv4, 4);

	conectar(&con);
	sprintf(query, "call spRegistrarIP(%lu, %u);", nueva_mac, nueva_ip);
	printf("MIRA NO MAS: %s", query);
	if(mysql_query(con, query)){
		error("No pude registrar la ip");
	}

	mysql_close(con);

	return 0;
}


int eliminar_ip(unsigned char * mac){
	if(mac == NULL){return -1;}

	MYSQL * con;
	char query[150];
	unsigned long nueva_mac = convertir_char_entero(mac, 6);

	conectar(&con);
	sprintf(query, "call spEliminarIP(%lu);", nueva_mac);

	if(mysql_query(con, query)){
		error("No pude eliminar la ip");
	}

	mysql_close(con);

	return 0;
}



int conectar(MYSQL ** con){
	if(con == NULL){return -1;}

	*con = mysql_init(NULL);

	if(mysql_real_connect(*con, "localhost", "root",
				"root", "dhcp", 0, NULL, 0) == NULL){
		error("No se pudo conectar a la base de datos.");
		return -1;
	}

	return 0;
}

int todo_nulo(struct conf_dhcp * conf){
	if(conf == NULL){return -1;}

	conf -> mac = NULL;
	conf -> ipv4 = NULL;
	conf -> mascara = NULL;
	conf -> enlace = NULL;
	conf -> dns = NULL;

	return 0;
}

void imprimir_direcciones(unsigned char * direccion, int tam, int tipo){
	if(direccion == NULL){return;}

	int i;
	char puntos;

	if(tipo == 0)
		puntos = ':';
	else
		puntos = '.';

	for(i = 0; i < tam; i++){
		if(tipo == 0)
			printf("%02x%c", (unsigned int)(direccion)[i], ((i < tam-1)?puntos:' '));
		else
			printf("%d%c", (unsigned int)(direccion)[i], ((i < tam-1)?puntos:' '));
	}
	printf("%s\n", " ");

}

void cerrar_config(struct conf_dhcp * conf){
	if(conf == NULL){return;}

	if(conf -> mac != NULL)
		free(conf -> mac);
	if(conf -> ipv4 != NULL)
		free(conf -> ipv4);
	if(conf -> mascara != NULL)
		free(conf -> mascara);
	if(conf -> enlace != NULL)
		free(conf -> enlace);
	if(conf -> mac != NULL)
		free(conf -> dns);

}

void error(char * msg){
	perror(msg);
	exit(1);
}
