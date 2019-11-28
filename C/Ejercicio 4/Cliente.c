#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <string.h>
#include <sys/shm.h>
#include <signal.h>
#include <ctype.h>

//CLIENTE
typedef struct
{
    int tipoConsulta;
    char valorConsulta[40];
    char consulta[60];
} Consulta;

typedef struct
{
    int id;
    char descripcion[40];
    char producto[40];
    char marca[40];
    //Consulta consulta;
} Articulo;

typedef struct
{
    Consulta consulta;
    Articulo articulo;
    int noResultado;
} SharedMemory;

void ayuda();
void * abrirMemoriaCompartida(const char *nombre, size_t size);

sem_t* crearSemaforo(const char* nombre, int valor);
sem_t* abrirSemaforo(const char *nombre);
void borrarSemaforo(const char * nombre, sem_t *semaforo);

void mostrarRegistro(Articulo art);

sem_t* semaforoServerOperando;
sem_t* semaforoClienteLee;
sem_t* semaforoClienteLeyo;
sem_t* semaforoClienteFinalizo;

int main(int argc, char const * argv[])
{
    sem_t* semaforoAcceso;
    sem_t* semaforoConsultaDepositada;
    SharedMemory* shm;
    char consulta[60];


    //Ayuda
    if(argc ==2 && (strcmp(argv[1],"-h")==0 || strcmp(argv[1], "-help")==0 || strcmp(argv[1], "-?")==0))
    {
        ayuda();
        return 1;
    }
    else if(argc!=2)
    {
        printf("Error: el cliente se ejecuta con un parámetro. Consulta la ayuda con el parámetro -h, -help o -?.\n");
        exit(2);
    }

    shm = abrirMemoriaCompartida("SharedMemory", sizeof(SharedMemory));
    if(shm == (void*)-1)
    {
        printf("No hay un servidor disponible, ejecute primero el servidor para poder operar.\n");
        exit(1);
    }

    semaforoConsultaDepositada = crearSemaforo("semaforoConsultaDepositada",0);
    semaforoAcceso = abrirSemaforo("semaforoAcceso");
    sem_wait(semaforoAcceso);
    //printf("Por favor, ingrese su consulta: ");
    //fgets(consulta,60,stdin);
    //sleep(1);
    strcpy(shm->consulta.consulta, argv[1]);
    sem_post(semaforoConsultaDepositada);
    semaforoServerOperando = abrirSemaforo("semaforoServerOperando");
    semaforoClienteLee = abrirSemaforo("semaforoClienteLee");
    semaforoClienteLeyo = abrirSemaforo("semaforoClienteLeyo");
    semaforoClienteFinalizo = abrirSemaforo("semaforoClienteFinalizo");
    int valorSemaforo;
    sem_getvalue(semaforoServerOperando,&valorSemaforo);
    sem_wait(semaforoClienteLee);
    if(shm->noResultado == 1)
    {
        while(valorSemaforo == 1)
        {
        mostrarRegistro(shm->articulo);
        sem_post(semaforoClienteLeyo);
        sem_wait(semaforoClienteLee);
        sem_getvalue(semaforoServerOperando,&valorSemaforo);
        };
    }
    sem_post(semaforoClienteFinalizo);
    //borrarSemaforo("semaforoConsultaDepositada",semaforoConsultaDepositada);
    return 0;
}

void* abrirMemoriaCompartida(const char *nombre, size_t size)
{
    int fd = shm_open(nombre, O_RDWR, 0);
    ftruncate(fd, size);

    void *dir = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    return dir;
}

sem_t* crearSemaforo(const char* nombre, int valor)
{
    //O_CREAT el semaforo se crea si no existe.
    //S_IRUSR y S_IWUSR permiten leer y escribir el semáforo.
    return sem_open(nombre, O_CREAT, S_IRUSR | S_IWUSR, valor); // 0600
}

sem_t* abrirSemaforo(const char *nombre)
{
    return sem_open(nombre, 0);
}

void borrarSemaforo(const char * nombre, sem_t *semaforo)
{
    sem_close(semaforo);
    sem_unlink(nombre);
}

void mostrarRegistro(Articulo art)
{
    printf("ID: %d\n", art.id);
    printf("Descripcion: ");
    puts(art.descripcion);
    printf("Producto: %s\n", art.producto);
    printf("Marca: ");
    puts(art.marca);
    printf("\n");
}

void ayuda()
{
    printf("Ejercicio 4 - Trabajo Práctico 3 - Segunda entrega\n");
    printf("\nIntegrantes:");
    printf("\n\tCarbone, Emanuel \t  40081161");
    printf("\n\tDe Stefano, Matias \t  40130248");
    printf("\n\tFiorita, Leandro \t  40012291");
    printf("\n\tGentile, Soledad \t  28053027");
    printf("\n\tPeralta, Julian \t  40242831\n\n");

    printf("Descripción:\n");
    printf("Se tiene una base de datos de articulos de un supermercado en un archivo de texto el cual se le pasa su dirección en el directorio actual del servidor.");
    printf("La identificación del articulo se hace a través de su número de ID. El archivo tiene el siguiente formato:\n");
    printf("ID;Descripción;Producto;Marca\n\n");
    printf("Las consultas se pueden realizar para cualquier campo.\n");
    printf("El formato de las consultas es el siguiente:\n");
    printf("Campo=Valor - Ejemplo: Producto=HELADO\n\n");
    printf("El servidor atiende las peticiones generadas desde un cliente.\n");
    printf("El cliente debe enviar por parametro la consulta a realizar.");
    printf("\nNota: este ejercicio utiliza memoria compartida.\n");

    printf("\nEjemplos de ejecución:");
    printf("\n./server\tarticulos.txt\n./cliente\t producto=helado\n");
}