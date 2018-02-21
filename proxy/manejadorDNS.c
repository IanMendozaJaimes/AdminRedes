#include "manejadorDNS.h"

int main(){

  unsigned char prueba[] = { /* Packet 423 */
      0x6e, 0xeb, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x03, 0x77, 0x77, 0x77,
      0x06, 0x67, 0x6f, 0x6f, 0x67, 0x6c, 0x65, 0x03,
      0x63, 0x6f, 0x6d, 0x00, 0x00, 0x01, 0x00, 0x01 };

  imprimirTrama(prueba, 32);
  crearRespuesta(prueba, 32);

  return 0;
}

int crearRespuesta(unsigned char * buffer, int tam){
  if(buffer == NULL){
    printf("%s\n", "El buffer no puede ser NULL");
    return -1;
  }

  struct infoTrama info;
  struct respuesta res;

  iniciarInfoTrama(&info);
  obtenerInfo(buffer, &info);

  printf("Nombre: %s\n", info.nombre);
  printf("Tam nombre: %d\n", info.tamNombre);
  printf("Tipo: %2x\n", info.tipo);

  iniciarRespuesta(buffer, tam, &res);
  imprimirTrama(res.datos, res.tam);

  if(strstr(info.nombre, "www")){
    short ptr = 0xc000;
    ptr += adjuntarCName(&res, &info, 0xc00c) & 0x3FFF;
    adjuntarARecord(&res, ptr);
  }
  else{
    adjuntarARecord(&res, 0xc00c);
  }

  printf("\n%s\n", "Respuesta final:");
  imprimirTrama(res.datos, res.tam);


  return 0;
}

int adjuntarCName(struct respuesta * res, struct infoTrama * info, short ptr){
  if(res == NULL){return -1;}

  unsigned char * record = (unsigned char *)malloc((13 + TAMPROHIBIDO + PARTESP)*sizeof(char));
  short tamNuevoNombre = TAMPROHIBIDO + PARTESP + 1;
  char nom[] = PROHIBIDO;
  int i, j, k, l, t, total=0;

  // ponemos un apuntador al nombre
  record[0] = (ptr >> 8) & 0xFF;
  record[1] = ptr & 0xFF;

  // ponemos el tipo
  record[2] = 0x00;
  record[3] = 0x05;

  // ponemos la clase
  record[4] = 0x00;
  record[5] = 0x01;

  // ponemos el ttl
  record[6] = 0x00;
  record[7] = 0x00;
  record[8] = 0x01;
  record[9] = 0x2b;

  // ponemos el tam
  record[10] = (tamNuevoNombre >> 8) & 0xFF;
  record[11] = tamNuevoNombre & 0xFF;

  // agregamos el nombre
  i = 12;
  j = 13;
  l = 0;
  for(i = 12, j = 13, k = 0; k < PARTESP; k++){
    t = 0;
    while(nom[l] != '.' && nom[l] != '\0'){
      record[j] = nom[l];
      j++;
      t++;
      l++;
    }
    record[i] = t;
    total += t + 1;
    i = j;
    j = i + 1;
    l++;
  }
  record[i] = 0;

  res -> datos = (unsigned char *)realloc(res -> datos, res -> tam + 13 + total);

  for(i = res -> tam, j = 0; i < res -> tam + 13 + total; i++, j++){
    (res -> datos)[i] = record[j];
  }

  res -> tam += total + 13;

  return (12 + info -> tamNombre + 1 + 4 + 12);

}

int adjuntarARecord(struct respuesta * res, short ptr){
  if(res == NULL){return -1;}

  unsigned char * record = (unsigned char *)malloc(17 * sizeof(char));
  int i, j;

  // ponemos un apuntador al nombre
  record[0] = (ptr >> 8) & 0xFF;
  record[1] = ptr & 0xFF;

  // ponemos el tipo
  record[2] = 0x00;
  record[3] = 0x01;

  // ponemos la clase
  record[4] = 0x00;
  record[5] = 0x01;

  // ponemos el ttl
  record[6] = 0x00;
  record[7] = 0x00;
  record[8] = 0x01;
  record[9] = 0x2b;

  // ponemos la longitud, debe ser 4 pues es un registro A
  record[10] = 0x00;
  record[11] = 0x04;

  // ponemos la direccion ip del servidor
  record[12] = 0x7f; // 127
  record[13] = 0x00;
  record[14] = 0x00;
  record[15] = 0x01;

  // ponemos el final de cadena
  record[16] = '\0';

  res -> datos = (unsigned char *)realloc(res -> datos, res -> tam + 16);

  for(i = res -> tam, j = 0; i < res -> tam + 16; i++, j++){
    (res -> datos)[i] = record[j];
  }

  res -> tam += 16;

  return 0;
}

int iniciarRespuesta(unsigned char * buffer, int tam, struct respuesta * res){
  if(buffer == NULL){return -1;}
  if(res == NULL){return -1;}

  int i = 0;
  res -> datos = (unsigned char *)malloc(tam * sizeof(unsigned char));
  for(; i < tam; i++){
    (res -> datos)[i] = buffer[i];
  }

  res -> tam = tam;

  return 0;
}

int obtenerInfo(unsigned char * buffer, struct infoTrama * info){
  if(buffer == NULL){return -1;}
  if(info == NULL){return -1;}

  int i = 12, j, tam = 0, totalTam = 0;

  while ((tam = buffer[i]) > 0) {
    char * aux = (char *)malloc((tam+2) * sizeof(char));
    for(j = 0; j < tam; j++){
      aux[j] = buffer[++i];
    }
    aux[j++] = '.';
    aux[j] = '\0';

    totalTam += tam+1;
    info -> nombre = (char *)realloc(info -> nombre, totalTam+1);
    strcat(info -> nombre, aux);
    info -> nombre[totalTam] = '\0';
    i++;
  }

  info -> tamNombre = totalTam;
  info -> tipo += ( (buffer[i+1] << 8) + buffer[i+2] ) & 0xFFFF;

  return 0;
}

int iniciarInfoTrama(struct infoTrama * info){
  if(info == NULL){return -1;}

  info -> nombre = NULL;
  info -> tamNombre = 0;
  info -> tipo = 0;

  return 0;
}

void imprimirTrama(unsigned char * trama, int tam){
  if(trama == NULL){return;}

  int i = 0;
  for(;i < tam;){
    if(trama[i] <= 0xF)
      printf("0%x ", trama[i]);
    else
      printf("%x ", trama[i]);
    if(++i % 16 == 0){
      printf("%s\n", "");
    }
    else if(i % 4 == 0){
      printf("%s", " ");
    }
  }

}
