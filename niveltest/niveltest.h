//Defines
#define BUFSIZE 1024
#define SERVER_IP 192.168.1.101
#define MAXLINE 1500 /*max text line length*/
#define MAXDATASIZE 100
#define MAXOBJAMOUNT 99
#define MAXNIVELNAME 12
#define HOSTIPLEN 21 /* IP:PORT*/
#define HOSTIP 16 /* IP */
#define MAXNUMERICLEN 3 /*0-999*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <commons/collections/list.h>
#include <commons/collections/node.h>
#include <commons/socket.h>
#include <commons/log.h>

//Definicion t_nivel

typedef struct nivel
{
	char* Nombre;
	char* Host;
	int Cajas; //cantidad de cajas
	int Puerto;
	int Enemigos; //cantidad enemigos
	int Sleep_Enemigos;
	int TiempoChequeoDeadlock;
	int Recovery; //indica si esta activa la eleccion de victima y su consecuente finalizacion

} t_nivel;
