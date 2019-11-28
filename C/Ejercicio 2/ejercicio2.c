/*###################################################################################
###                               INICIO DE SCRIPT                                ###
#Nombre del script: ejercicio2.c
#TP: C
#Número de ejercicio: 2
#Integrantes:
# Carbone, Emanuel - DNI: 40081161      
# De Stefano, Matías - DNI: 40130248  
# Peralta, Julián - DNI: 40242831
#Entrega: Primera reentrega
###################################################################################*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#define TAM 1500
#define ARCHIVO_RESULTADOS "resultados.txt"

typedef struct {
int orden, 
    hiloTratado;
double vector1, 
       vector2, 
       resultado; 
}tVectores;

int esDirectorio(const char* ruta);
int leerArchivo(FILE *, char *, int);
void * sumarVectores(void *vVectores);
void getHelp();

int main(int argc, char *argv[]){

    FILE *fp, *fpEscribir;
    char *archivoVectores, linea[TAM];
    int cantHilos, cantVectores = 0, hilo = 0, indice = 0;
    double vector1, vector2;
    pthread_t *hilos;
    pthread_attr_t atributos;

    if(argc == 1 || argc >= 4 || (argc == 3 && atoi(argv[2]) <= 0)){
        printf("\nError en los parámetros! Ejecute el script con -h para obtener ayuda.\n\n");
        return 1;
    }

    if(argc == 2){
        if(strcmp(argv[1], "-h") == 0){
            getHelp();
            return 0;
        }
        else{
            printf("\nError en los parámetros! Ejecute el script con -h para obtener ayuda.\n\n");
            return 1;
        }
    }

    if(esDirectorio(argv[1]) == 0){
        printf("\nError en los parámetros! \"%s\" no es un archivo regular. \n\n", argv[1]);
        return 1;
    }
    
    archivoVectores = argv[1];
    cantHilos = atoi(argv[2]);

    fp = fopen(archivoVectores, "rt");

    if(!fp){
        printf("\n%s no se pudo abrir.\n", archivoVectores);
        return 1;
    }
    
    if(access(ARCHIVO_RESULTADOS, F_OK) != -1) {   
        if(remove(ARCHIVO_RESULTADOS) != 0){
            printf("No es posible borrar el archivo de resultados: %s.", ARCHIVO_RESULTADOS);
        return 1;
        }
    }
    
    tVectores vectores[TAM];

    hilos=(pthread_t*)malloc(sizeof(pthread_t)*cantHilos);
    pthread_attr_init(&atributos);
    pthread_attr_setdetachstate(&atributos, PTHREAD_CREATE_JOINABLE);

    while(leerArchivo(fp, linea, sizeof(linea))){
        
        if((indice % cantHilos == 0) || cantHilos == 1)
            hilo = 0;

        sscanf(linea, "%lf,%lf", &vector1, &vector2);
                
        vectores[indice].orden = indice;
        vectores[indice].hiloTratado = hilo;
        vectores[indice].vector1 = vector1;
        vectores[indice].vector2 = vector2;

        indice++;
        hilo++;
    }
    
    fclose(fp);

    hilo = 0;
    if(cantHilos > indice)
        cantHilos = indice;

    for(int i = 1; i < indice; i++ ){

        if((i % cantHilos == 0) || cantHilos == 1)
            hilo = 0;
        
        pthread_create(hilos+hilo, &atributos, &sumarVectores, (void*) &vectores[i]);
        
        if(hilo == cantHilos-1 || i == indice-1){
            for(int i=0; i < cantHilos; i++){
                pthread_join(*(hilos+i), NULL);
            }
        }
        hilo++;
    }

    fpEscribir = fopen(ARCHIVO_RESULTADOS, "a");

    if(!fpEscribir)
        printf("No se pudo abrir el archivo de resultados: %s.\n", ARCHIVO_RESULTADOS);

    int distrHilos[cantHilos];

    for(int i = 0; i < cantHilos; i++) 
       distrHilos[i] = 0;

    for(int i = 0; i < indice; i++){

        fprintf(fpEscribir, 
        "Número de par: %d | Hilo: %d | Vector 1: %f | Vector 2: %f | Resultado: %f\n", 
        vectores[i].orden, vectores[i].hiloTratado, vectores[i].vector1, vectores[i].vector2, vectores[i].resultado);

        distrHilos[vectores[i].hiloTratado]++;
    }

    fclose(fpEscribir);
    free(hilos);

    for(int i = 0; i < cantHilos; i++)
        printf("\nEl hilo %d sumó %d pares de vectores.\n", i, distrHilos[i]++);

    printf("\nEjecución finalizada. Los resultados se encuentran en %s.\n\n", ARCHIVO_RESULTADOS);
    return 0;            
}

int esDirectorio(const char* ruta){
    struct stat path_stat;
    stat(ruta, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

int leerArchivo(FILE *fp, char *linea, int tam){

    char *aux;

    if(fgets(linea, tam, fp))
    {
        aux = strchr(linea, '\n');
        if(aux == NULL)
            aux = strchr(linea, '\0');
        *aux = '\0';

        return 1;
    }
    else
        return 0;
} 

void * sumarVectores(void *vVectores){

    tVectores *vectores = (tVectores*)vVectores;
    vectores->resultado = vectores->vector1 + vectores->vector2;

    return vectores;
}

void getHelp(){
    printf("\nNombre del ejecutable: ejercicio2\n");
    printf("\n-------------------------------------------------------------------------------------------\n");
    printf("\nDescripción:\n");
    printf("\n-El programa reparte todos los pares de vectores del archivo de entrada");
    printf("\n en forma equitativa entre los N threads que se han solicitado.");
    printf("\n\n-En el archivo de salida llamado \"%s\", se encontrarán los resultados del", ARCHIVO_RESULTADOS);
    printf("\n procesamiento de los vectores, indicando el hilo que trató al par de vectores,");
    printf("\n la composición del mismo y el resultado de la suma entre ambos.");
    printf("\n\n-También se muestra por pantalla cómo es la distribución de los vectores entre los hilos.");
    printf("\n-------------------------------------------------------------------------------------------\n");
    printf("\nParámetros:\n");
    printf("\nPrimer parametro: path a directorio de entrada.");
    printf("\nSegundo parametro: cantidad de hilos.");
    printf("\n-------------------------------------------------------------------------------------------\n");
    printf("\nEjemplo de ejecución:\n");
    printf("\n./ejercicio2 vectores.txt 3");
    printf("\n-------------------------------------------------------------------------------------------\n");
}

/*###################################################################################
###                               FIN DE SCRIPT                                   ###
###################################################################################*/