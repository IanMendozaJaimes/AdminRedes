#include "servidor_dhcp.h"

int ingresar_datos(struct conf_dhcp *);
int llenar_mac(struct conf_dhcp *, unsigned char, int *, int *);

int main(int size, const char * args[]){

  if(size < 2){
    printf("%s\n", "Tienes que poner como parametro la ruta de las configuraciones en txt.");
    exit(1);
  }

  int archivo, leidos, estado, contador, otro;
  unsigned char aux;
  struct conf_dhcp conf;

  archivo = open(args[1], O_RDONLY);
  todo_nulo(&conf);
  estado = 1;
  otro = 0;
  contador = 0;

  while((leidos = read(archivo, &aux, 1)) > 0){
    if((aux > 96 && aux < 103) || (aux > 47 && aux < 58)){
      if(estado == 1)
        llenar_mac(&conf, aux, &contador, &otro);
      else if(estado == 2){ // ipv4
        conf.ipv4 = (unsigned char *)malloc(sizeof(char));
        (conf.ipv4)[0] = aux - 48;
      }
      else if(estado == 3){
        conf.dns = (unsigned char *)malloc(sizeof(char));
        (conf.dns)[0] = aux - 48;
      }
      else if(estado == 4){
        conf.enlace = (unsigned char *)malloc(sizeof(char));
        (conf.enlace)[0] = aux - 48;
      }
      else if(estado == 5){
        conf.mascara = (unsigned char *)malloc(sizeof(char));
        (conf.mascara)[0] = aux - 48;
      }
    }
    else if(aux == 9){
      estado++;
    }
    else if(aux == 10){
      ingresar_datos(&conf);
      cerrar_config(&conf);
      todo_nulo(&conf);
      contador = 0;
      otro = 0;
      estado = 1;
    }
  }

  return 0;
}


int ingresar_datos(struct conf_dhcp * conf){
  if(conf == NULL){return -1;}

  MYSQL * con;
  char query[120];

  int i = 0;
	for(;i < 6;){
		printf("%02x ", (conf -> mac)[i]);
		if(++i % 16 == 0){
			printf("\n");
		}
		else if(i % 8 == 0){
			printf(" ");
		}
	}
	printf("\n");

  unsigned long nueva_mac = convertir_char_entero(conf -> mac, 6);
  printf("%lu\n", nueva_mac);

  conectar(&con);

  sprintf(query, "call spIngresarDatos(%lu, %u, %u, %u, %u);",
    nueva_mac, (conf -> ipv4)[0], (conf -> dns)[0],
    (conf -> enlace)[0], (conf -> mascara)[0]);

  if(mysql_query(con, query)){
    error("No pude registrar la nueva configuracion");
  }

  return 0;
}


int llenar_mac(struct conf_dhcp * conf, unsigned char aux, int * contador, int * otro){
  if(conf == NULL){return -1;}

  printf("%c\n", aux);

  if(conf -> mac == NULL)
    conf -> mac = (unsigned char *)malloc(6 * sizeof(char));

  if(aux > 96)
    aux -= 87;
  else
    aux -= 48;

  if(*otro == 0){
    (conf -> mac)[*contador] =  0;
    (conf -> mac)[*contador] +=  aux << 4;
    *otro += 1;
  }
  else{
    (conf -> mac)[*contador] +=  aux;
    *otro = 0;
    *contador += 1;
  }

  return 0;
}
