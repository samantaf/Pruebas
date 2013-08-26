/*
 ============================================================================
 Name        : Test1.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

int compararEnteros(void);

int main (void){
	compararEnteros();
	printf("Termino la comparacion\n");

	return 0;
}

int compararEnteros(void) {
	int num1;
	int num2;
	printf("Introduzca 2 entero y le dire que relacion mantienen: ");

	scanf("%d%d", &num1, &num2);

	if (num1 == 0 && num2 == 0){return 0;}

	if (num1 != num2){
		printf("%d no es igual que %d\n", num1, num2);
	} else {printf("%d es igual que %d\n", num1, num2);}

	if (num1 <= num2){
			printf("%d es menor o igual que %d\n", num1, num2);
		}

	if (num1 >= num2){
			printf("%d es mayor o igual que %d\n", num1, num2);
		}

	compararEnteros();
	return 0;
}
