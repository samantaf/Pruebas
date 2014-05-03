#ifndef __NIVEL_H__

#define __NIVEL_H__

#define PERSONAJE_ITEM_TYPE 0
#define RECURSO_ITEM_TYPE 1
#define ENEMIGO_ITEM_TYPE 2

#include <niveltest.h>
#include <sys/ioctl.h>
#include <commons/string.h>
#include <commons/config.h>





typedef struct item {
	char id;
	int posx;
	int posy;
	char item_type; // PERSONAJE o CAJA_DE_RECURSOS
	int quantity;
} ITEM_NIVEL;

ITEM_NIVEL* item_create(char id, int cant_rec, int x , int y, char tipo);
ITEM_NIVEL* buscarItem(t_list* ListaItems, char id);
void borrarItem(t_list* ListaItems, char id);
int activarOcultarItem(t_list* ListaItems, char id, int enabled);

int nivel_gui_dibujar(t_list* items);

void MoverPersonaje(t_list* ListaItems, char id, int x, int y) ;

void CrearItem(ITEM_NIVEL** ListaItems, char id, int x , int y, char tipo, int cant_rec);


int nivel_gui_dibujar(t_list* items, char* nombre_nivel);
int nivel_gui_terminar(void);
int nivel_gui_inicializar(void);
int nivel_gui_get_area_nivel(int * filas, int * columnas);

#endif
