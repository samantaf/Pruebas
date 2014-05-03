#include "niveltest.h"
#include <nivel.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include "errno.h"
#include <signal.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <commons/get_ip.h>

//Variables globales
	int cantidadRecursos;
	int* recursosSimboloPosicion;
	int* recursosDisponiblesTotales;

// Estructura Global para cada NIVEL
	t_nivel nivel;

// Lista global de Items/Personajes
	t_list* ListaItems = NULL;
