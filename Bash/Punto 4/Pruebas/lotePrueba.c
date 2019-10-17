#using stdio.h

int main()
{
	int a; //Primer variable a sumar
	int b; //Segunda variable a sumar
	int resultado; /* Variable donde se almacenara el resultado*/

	/* a=30;
	b = 20;
	*/

	/*Los valores de arriba
	no se utilizan mas*/

	a = 7;
	b = 5;

	resultado = a + b;

	/* Básicamente esto, debería funcionar. //Es decir, bien.
	//Excelente */

	printf("El resultado es: %d", &resultado); //Esta funcion muestra el resultado de la suma
	return 0;
}
