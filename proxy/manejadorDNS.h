#ifndef  __MANEJADORDNS__
#define  __MANEJADORDNS__

  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>

  #define MAXBUFFER 1024
  #define PROHIBIDO "google.com"
  #define TAMPROHIBIDO 10
  #define PARTESP 2

  // aqui podemos almacenar informacion de utilidad
  struct infoTrama{
    char * nombre;
    int tamNombre;
    int tipo;
  };

  struct respuesta{
    unsigned char * datos;
    int tam;
  };

  int iniciarInfoTrama(struct infoTrama *);
  int crearRespuesta(unsigned char *, int);
  int obtenerInfo(unsigned char *, struct infoTrama *);
  void imprimirTrama(unsigned char *, int);
  int iniciarRespuesta(unsigned char *, int, struct respuesta *);
  int adjuntarARecord(struct respuesta *, short);
  int adjuntarCName(struct respuesta *, struct infoTrama *, short);

#endif
