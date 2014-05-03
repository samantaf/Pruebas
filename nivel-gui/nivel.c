// solo para agregar los .proyect

#include "nivel.h"

static WINDOW * secwin;
static WINDOW * mainwin;
static int rows, cols;
static int inicializado = 0;


int CargarConfFile(char* archivo, t_nivel* nivel){

	int LogPantallaInt;
	// EL DICCIONARIO SE CREA Y SE DESTRUYE DENTRO DE ESTA FUNCION
	// aca mismo ya se setea la estructura y la lista

	t_config* config = config_create(archivo);


	if (config->properties->elements_amount==0){ //Si esta vacio va a tirar error
		perror ("Error al abrir el archivo");
		return EXIT_FAILURE;
	}

	int k,i;
	// calloc es necesario dar el número de elementos a reservar y el tamaño de cada uno de esos elementos
	char* orquestador = (char*)calloc(21,sizeof(char));
	char* caja = (char*)calloc(8,sizeof(char));				// Caja1
	char* number = (char*)calloc(2,sizeof(char));			// 1

	char* linea = (char*)calloc(50,sizeof(char));			// "Flores,F,1,5,7"
	char **array_values = (char**)calloc(30,sizeof(char));	// [Flores,F,1,5,7]

	//PORQUEE??

	linea = strstr(config_get_string_value(config,"Nombre"), "/"); 		// El nombre no puede contener '/'
	if(linea!=NULL){
		perror ("Nombre de Nivel invalido");
		return EXIT_FAILURE;
	}
	linea = NULL;
	linea = (char*)calloc(50,sizeof(char));

	strcpy(nivel->Nombre , config_get_string_value(config,"Nombre"));
	strcpy(orquestador , config_get_string_value(config,"orquestador"));
	strcpy(nivel->Host , strtok( orquestador, ":\0")); //separo el host del Puerto
	nivel->Puerto = atoi(strtok( NULL, ":\0" ));
	nivel->TiempoChequeoDeadlock = config_get_int_value(config,"TiempoChequeoDeadlock");
	nivel->Enemigos= config_get_int_value(config,"Enemigos")

	if(config_has_property(config,"Recovery")) nivel->Recovery = config_get_int_value(config,"Recovery");
	else nivel->Recovery = 1; // Tomo un valor por default

	//Busco la cantidad total de cajas de recursos
	k=1;
	sprintf(number,"%d",k);
	strcpy(caja , "Caja");
	string_append(&caja,number); //concatena la caja con el numero
	while(config_has_property(config,caja)){
		k++;
		sprintf(number,"%d",k);
		strcpy(caja , "Caja");
		string_append(&caja,number);
	}
	nivel->Cajas = (k-1);

	for(k=1;k<=(nivel->Cajas);k++){
		i=0;
		sprintf(number,"%d",k);
		strcpy(caja , "Caja");
		string_append(&caja,number);
		//printf("%s\n",caja);
		strcpy(linea, config_get_string_value(config,caja));	// "Flores,F,1,5,7"
		//printf("Linea = %s\n", linea);
		array_values = string_split(linea, ",");		// [Flores,F,1,5,7]
	    while (array_values[i] != NULL) {
		    string_trim(&(array_values[i]));  //elimina los espacios vacios de izquierda y derecha
			//printf("item n° %d = %s\n", i, array_values[i]);
		    i++;
	    }
	    list_add(ListaItems, (void*)item_create(*array_values[1], atoi(array_values[2]), atoi(array_values[3]), atoi(array_values[4]), RECURSO_ITEM_TYPE));

		free(array_values);
		linea = NULL;
		linea = (char*)calloc(50,sizeof(char));
	}

	free(number);
	free(caja);
	free(orquestador);
	free(linea);

	config_destroy(config); //Destruye el diccionario

	return EXIT_SUCCESS;
}

//Hasta aca llegue 31/10, falta ver en que estructura van a estar los enemigos e ir guardandolos a medida que los lee del archivo de config

// ------ Prototipos de Funciones utilitarias ------------

void nivel_gui_get_term_size(int * filas, int * columnas);
int nivel_gui_int_validar_inicializado(void);
void nivel_gui_print_perror(const char* mensaje);

// ------------------------------------------------------

/*
 * @NAME: nivel_gui_inicializar
 * @DESC: Inicializa el espacio de dibujo
 */
int nivel_gui_inicializar(void) {

	if (nivel_gui_int_validar_inicializado()){
                nivel_gui_print_perror("nivel_gui_inicializar: Library ya inicializada!");
                return EXIT_FAILURE;
        }

	mainwin = initscr();
	keypad(stdscr, TRUE);
	noecho();
	start_color();
	init_pair(1,COLOR_GREEN, COLOR_BLACK);
	init_pair(2,COLOR_WHITE, COLOR_BLACK);
	init_pair(3,COLOR_BLACK, COLOR_YELLOW);
	init_pair(4,COLOR_BLACK, COLOR_BLUE);
	box(stdscr, 0, 0);
	refresh();

	nivel_gui_get_term_size(&rows, &cols);
	secwin = newwin(rows - 3, cols, 0, 0);
	box(secwin, 0, 0);
	wrefresh(secwin);

	inicializado = 1;

	return EXIT_SUCCESS;

}

/*
 * @NAME: nivel_gui_dibujar
 * @DESC: Dibuja cada entidad en la lista de items
 * @PARAMS:
 * 		items	  - lista de objetos a dibujar
 * 		nombre_nivel - nombre del nivel
 */
int nivel_gui_dibujar(t_list* items, char* nombre_nivel) {

	if (!nivel_gui_int_validar_inicializado()){
		nivel_gui_print_perror("nivel_gui_dibujar: Library no inicializada!");
		return EXIT_FAILURE;
	}

	if (items == NULL){
		nivel_gui_print_perror("nivel_gui_dibujar: La lista de items no puede ser NULL");
		return EXIT_FAILURE;
	}

	int i = 0;
	werase(secwin);
	box(secwin, 0, 0);
	wbkgd(secwin, COLOR_PAIR(1));

	move(rows - 3, 2);
	printw("Nivel: %s", nombre_nivel);
	move(rows - 2, 2);
	printw("Recursos: ");

        
        void _draw_element(ITEM_NIVEL* item) {
            wmove(secwin, item->posy, item->posx);
            if(item->item_type == ENEMIGO_ITEM_TYPE) {
                    waddch(secwin, '*' | COLOR_PAIR(4));
            } else if (item->item_type == RECURSO_ITEM_TYPE) {
                    waddch(secwin, item->id | COLOR_PAIR(3));
            } else if(item->item_type == PERSONAJE_ITEM_TYPE) {
                    waddch(secwin, item->id | COLOR_PAIR(2));
            }
            if (item->item_type == RECURSO_ITEM_TYPE) {
                move(rows - 2, 7 * i + 3 + 9);
                printw("%c: %d - ", item->id, item->quantity);
                i++;
            }
        }
        
        list_iterate(items, (void*) _draw_element);
       
	wrefresh(secwin);
	wrefresh(mainwin);

	return EXIT_SUCCESS;

}

/*
 * @NAME: nivel_gui_terminar
 * @DESC: Termina el nivel de forma prolija
 */
int nivel_gui_terminar(void) {

        if (!nivel_gui_int_validar_inicializado()){
                nivel_gui_print_perror("nivel_gui_terminar: Library no inicializada!");
                return EXIT_FAILURE;
        }

        delwin(mainwin);
        delwin(secwin);
        endwin();
        refresh();

        return EXIT_SUCCESS;

}

/*
 * @NAME: nivel_gui_get_area_nivel
 * @DESC: Devuelve el tamanio de la pantalla usable
 * @PARAMS:
 * 		rows - valor de retorno de filas
 * 		cols - valor de retorno de columnas
 */
int nivel_gui_get_area_nivel(int * rows, int * cols) {

	if (!nivel_gui_int_validar_inicializado()){
			nivel_gui_print_perror("nivel_gui_get_area_nivel: Library no inicializada!");
			return EXIT_FAILURE;
			}

	if (rows == NULL || cols == NULL){
		nivel_gui_print_perror("nivel_gui_get_area_nivel: Ninguno de los argumentos puede ser NULL");
		return EXIT_FAILURE;
	}


	nivel_gui_get_term_size(rows, cols);
	*rows = *rows - 5;
	*cols = *cols - 2;

	return EXIT_SUCCESS;
}

/*
 * @NAME: nivel_gui_get_term_size
 * @DESC: Devuelve el tamanio total de la pantalla
 * @PARAMS:
 * 		rows - valor de retorno de filas
 * 		cols - valor de retorno de columnas
 */
void nivel_gui_get_term_size(int * rows, int * cols) {

    struct winsize ws;

    if ( ioctl(0, TIOCGWINSZ, &ws) < 0 ) {
        perror("couldn't get window size");
    }

    *rows = ws.ws_row;
    *cols = ws.ws_col;
}

/*
 * @NAME: nivel_gui_int_validar_inicializado
 * @DESC: Informa si se inicializo el nivel corectamente
 */
int nivel_gui_int_validar_inicializado(void){
	return inicializado;
}

/*
 * @NAME: nivel_gui_print_perror
 * @DESC: Imprime un error en el nivel
 * @PARAMS:
 * 		message - mensaje a imprimir
 */
void nivel_gui_print_perror(const char* message){
	fprintf(stderr, "%s\n", message);
}



