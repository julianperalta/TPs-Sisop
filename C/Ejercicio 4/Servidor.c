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

///
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>


//SERVER

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

//Funciones
static void skeleton_daemon();
int checkDirectorio(const char* ruta);
void ayuda();
void* crearMemoriaCompartida(const char* nombre, size_t size);
void borrarMemoriaCompartida(const char*, size_t, void*);
sem_t* crearSemaforo(const char* nombre, int valor);
sem_t* abrirSemaforo(const char *nombre);
void borrarSemaforo(const char*, sem_t*);



Consulta identificarConsulta(char* consulta);
void terminar(int signum);
void leerArchivo(char* nomArchivo);
void mostrarRegistro(Articulo art);
void eliminarNewline(char* linea);
void eliminarNewlineN(char* linea);
int strcmpi(char const *a, char const *b);
void buscarID(char* IDABuscar, char* nomArchivo);
void buscarDescripcion(char* descABuscar, char* nomArchivo);
void buscarProducto(char* producto, char* nomArchivo);
void buscarMarca(char* marca, char* nomArchivo);

SharedMemory* shm;
sem_t* semaforoAcceso;
sem_t* semaforoConsultaDepositada;
sem_t* semaforoServerOperando;
sem_t* semaforoClienteLee;
sem_t* semaforoClienteLeyo;
sem_t* semaforoClienteFinalizo;
char rutaArchivo[200];

int main(int argc, char const * argv[])
{
    skeleton_daemon();
    signal(SIGINT,terminar);
    signal(SIGTERM, terminar);
    //Ayuda
    if(argc ==2 && (strcmp(argv[1],"-h")==0 || strcmp(argv[1], "-help")==0 || strcmp(argv[1], "-?")==0))
    {
        ayuda();
        return 1;
    }
    else if(argc!=2 )
    {
        printf("Error: el servidor se ejecuta con un parámetro. Consulta la ayuda con el parámetro -h, -help o -?.\n");
        exit(2);
    }
    strcpy(rutaArchivo, argv[1]);
    if(checkDirectorio(rutaArchivo) == 0)
    {
        printf("El directorio indicado no es un archivo regular.");
        terminar(SIGINT);
    }
    printf("Servidor en ejecucion...\n");
    semaforoClienteFinalizo = crearSemaforo("semaforoClienteFinalizo",0);
    semaforoAcceso = crearSemaforo("semaforoAcceso", 1);
    shm = (SharedMemory*) crearMemoriaCompartida("SharedMemory",sizeof(SharedMemory));
    semaforoServerOperando = crearSemaforo("semaforoServerOperando", 1);
    semaforoClienteLee = crearSemaforo("semaforoClienteLee", 0);
    semaforoClienteLeyo = crearSemaforo("semaforoClienteLeyo", 0);

    int valorSemaforo;
    sem_getvalue(semaforoAcceso, &valorSemaforo);
    do
    {

        if(valorSemaforo == 1)
            sem_getvalue(semaforoAcceso, &valorSemaforo);
        else
        {
            semaforoConsultaDepositada = abrirSemaforo("semaforoConsultaDepositada");
            sem_wait(semaforoConsultaDepositada);
            shm->consulta = identificarConsulta(shm->consulta.consulta);

            switch(shm->consulta.tipoConsulta)
            {
            case 1:
                buscarID(shm->consulta.valorConsulta, rutaArchivo);
                break;

            case 2:
                buscarDescripcion(shm->consulta.valorConsulta, rutaArchivo);
                break;

            case 3:
                buscarProducto(shm->consulta.valorConsulta, rutaArchivo);
                break;

            case 4:
                buscarMarca(shm->consulta.valorConsulta, rutaArchivo);
                break;

            case 0:
                buscarMarca(shm->consulta.valorConsulta, rutaArchivo);
                //printf("Se ha ingresado una consulta invalida.\n");
                //terminar(SIGINT);
            }
            sem_wait(semaforoClienteFinalizo);
            sem_post(semaforoAcceso);
            sem_post(semaforoServerOperando);
        }

    }
    while(1);
    return 0;
}


//Si tipoConsulta == 0 entonces hay un error.
Consulta identificarConsulta(char* consulta)
{
    Consulta retorno;
    char* valor;
    char* campo;
    char copiaConsulta[60];
    strcpy(copiaConsulta,consulta);

    valor = strchr(consulta,'=');
    if(valor == NULL)
    {
        retorno.tipoConsulta = 0;
        strcpy(retorno.valorConsulta, "0000\n");
        return retorno;
    }
    valor++;
    campo = strtok(copiaConsulta, "=");
    if(campo == NULL)
    {
        retorno.tipoConsulta = 0;
        strcpy(retorno.valorConsulta, "0000\n");
        return retorno;
    }
    if(strcmpi(campo,"id") == 0)
        retorno.tipoConsulta = 1;
    else if(strcmpi(campo,"descripcion") == 0)
        retorno.tipoConsulta = 2;
    else if(strcmpi(campo,"producto") == 0)
        retorno.tipoConsulta = 3;
    else if(strcmpi(campo,"marca") == 0)
        retorno.tipoConsulta = 4;
    else
    {
        retorno.tipoConsulta = 0;
        strcpy(retorno.valorConsulta, "0000\n");
        return retorno;
    }
    strcpy(retorno.valorConsulta, valor);
    strcpy(retorno.consulta, "null");
    return retorno;
}

void terminar(int signum)
{
    printf("\nSaliendo...\n");
    borrarMemoriaCompartida("SharedMemory", sizeof(SharedMemory), shm);
    borrarSemaforo("semaforoAcceso", semaforoAcceso);
    borrarSemaforo("semaforoConsultaDepositada", semaforoConsultaDepositada);
    borrarSemaforo("semaforoServerOperando", semaforoServerOperando);
    borrarSemaforo("semaforoClienteLee", semaforoClienteLee);
    borrarSemaforo("semaforoClienteLeyo", semaforoClienteLeyo);
    borrarSemaforo("semaforoClienteFinalizo", semaforoClienteFinalizo);
    exit(1);
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

void * crearMemoriaCompartida(const char *nombre, size_t size)
{
    int fd = shm_open(nombre, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    ftruncate(fd, size);

    void *dir = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    return dir;
}

void borrarMemoriaCompartida(const char *nombre, size_t size, void *direccion)
{
    munmap(direccion, size);
    shm_unlink(nombre);
}

void leerArchivo(char* nomArchivo)
{
    Articulo art;

    char idString[10];
    FILE *fp = fopen(nomArchivo, "r") ;
    if (!fp)
    {
        printf("No existe el archivo.\n");
        terminar(SIGINT);
        return;
    }

    while (fscanf(fp,"%[^;];%[^;];%[^;];%[^\n]", idString, art.descripcion, art.producto, art.marca) && !feof(fp))
    {
        art.id = atoi(idString);
        mostrarRegistro(art);
    }
    return;
}

void buscarID(char* IDABuscar, char* nomArchivo)
{
    Articulo art;

    int idSearch;
    char idString[10];
    FILE *fp = fopen(nomArchivo, "r") ;
    if (!fp)
    {
        printf("No existe el archivo.\n");
        terminar(SIGINT);
        return;
    }
    shm->noResultado = 0;
    eliminarNewlineN(IDABuscar);
    idSearch = atoi(IDABuscar);
    while (fscanf(fp,"%[^;];%[^;];%[^;];%[^\n]", idString, art.descripcion, art.producto, art.marca) && !feof(fp))
    {
        art.id = atoi(idString);
        if(art.id == idSearch)
        {
            shm->noResultado = 1;
            shm->articulo = art;
            sem_post(semaforoClienteLee);
            sem_wait(semaforoClienteLeyo);
        }
    }
    sem_wait(semaforoServerOperando);
    sem_post(semaforoClienteLee);
    return;
}

void buscarDescripcion(char* descABuscar, char* nomArchivo)
{
    Articulo art;

    char idString[10];
    FILE *fp = fopen(nomArchivo, "r") ;
    if (!fp)
    {
        printf("No existe el archivo.\n");
        terminar(SIGINT);
        return;
    }
    shm->noResultado = 0;
    eliminarNewlineN(descABuscar);
    while (fscanf(fp,"%[^;];%[^;];%[^;];%[^\n]", idString, art.descripcion, art.producto, art.marca) && !feof(fp))
    {
        art.id = atoi(idString);
        if(strcmpi(descABuscar,art.descripcion) == 0)
        {
            shm->noResultado = 1;
            shm->articulo = art;
            sem_post(semaforoClienteLee);
            sem_wait(semaforoClienteLeyo);
        }
    }
    sem_wait(semaforoServerOperando);
    sem_post(semaforoClienteLee);
    return;
}

void buscarProducto(char* producto, char* nomArchivo)
{
    Articulo art;

    char idString[10];
    FILE *fp = fopen(nomArchivo, "r") ;
    if (!fp)
    {
        printf("No existe el archivo.\n");
        terminar(SIGINT);
        return;
    }
    shm->noResultado = 0;
    eliminarNewlineN(producto);
    while (fscanf(fp,"%[^;];%[^;];%[^;];%[^\n]\n", idString, art.descripcion, art.producto, art.marca) && !feof(fp))
    {
        art.id = atoi(idString);
        if(strcmpi(producto,art.producto) == 0)
        {
            shm->noResultado = 1;
            shm->articulo = art;
            sem_post(semaforoClienteLee);
            sem_wait(semaforoClienteLeyo);
        }
    }
    sem_wait(semaforoServerOperando);
    sem_post(semaforoClienteLee);
    return;
}

void buscarMarca(char* marcaBusqueda, char* nomArchivo)
{
    Articulo art;

    char idString[10];
    FILE *fp = fopen(nomArchivo, "r") ;
    if (!fp)
    {
        printf("No existe el archivo.\n");
        terminar(SIGINT);
        return;
    }
    shm->noResultado = 0;
    eliminarNewlineN(marcaBusqueda);

    while (fscanf(fp,"%[^;];%[^;];%[^;];%[^\n]", idString, art.descripcion, art.producto, art.marca) && !feof(fp))
    {
        eliminarNewline(art.marca);
        art.id = atoi(idString);
        if(strcmpi(marcaBusqueda,art.marca) == 0)
        {
            shm->noResultado = 1;
            shm->articulo = art;
            sem_post(semaforoClienteLee);
            sem_wait(semaforoClienteLeyo);
        }
    }
    sem_wait(semaforoServerOperando);
    sem_post(semaforoClienteLee);
    return;
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

void eliminarNewline(char* linea)
{
    char new_char = '\0';
    char* pFirstN = strstr(linea, "\r");
    *pFirstN = new_char;
}

void eliminarNewlineN(char* linea)
{
    char new_char = '\0';
    char* pFirstN = strstr(linea, "\n");
    if(pFirstN != NULL)
        *pFirstN = new_char;
    return;
}

int strcmpi(char const *a, char const *b)
{
    for (;; a++, b++)
    {
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if (d != 0 || !*a)
            return d;
    }
}

static void skeleton_daemon()
{
    pid_t pid;

    //Forkea el proceso padre.
    pid = fork();

    //Ocurrió un error.
    if (pid < 0)
        exit(EXIT_FAILURE);

    //Éxito, hacer que el padre termine.
    if (pid > 0)
        exit(EXIT_SUCCESS);

    //El hijo se convierte en el líder de la sesión.
    if (setsid() < 0)
        exit(EXIT_FAILURE);
}

int checkDirectorio(const char* ruta)
{
    struct stat path_stat;
    stat(ruta, &path_stat);
    return S_ISREG(path_stat.st_mode);
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