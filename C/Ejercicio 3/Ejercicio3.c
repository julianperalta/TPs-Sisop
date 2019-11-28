/**
 * INICIO DEL SCRIPT
 *
 * Nombre del Script: Ejercicio3.c
 * TP: C
 * Numero de Ejercicio: 3
 * Integrantes:
 *  Carbone Emanuel 40081171
 *  De Stefano, Matias 40130248
 *  Fiorita, Leandro 40012291
 *  Gentile, Soledad 28053027
 *  Peralta, Julian 40242831
 *
 * Entrega: Segunda Entrega
 **/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#define TAM 80

/*
Tabla con lista de artículos
texto plano separado por ;
id; desc; prod; marca

proceso al que se puede enviar consulta desde otros procesos o comandos
devolver datos del articulo.

Demonio lee FIFO consultas y devuelve FIFO encontrados.

Param: ruta, nombre de fifo y crearlo si es necesario

consulta ej: ID=123455
*/

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
} Articulo;


void intercambiar_cad(char *) ;
char* itoa(int, char*);
void crearLinea(Articulo art, char *);
int strcmpi(const char *a, const char *b);
void ayuda();
void crearDemonio();
Consulta identificarConsulta(char* consulta);
void procesarConsulta(Consulta consulta, const char* rutaArchivo);
Articulo leerArchivo(const char *arch, char *busqueda);
void terminar(int signum);
void crear_demonio();

void buscarID(char* IDABuscar, const char* nomArchivo);
void buscarMarca(char* marcaBusqueda, const char* nomArchivo);
void buscarProducto(char* producto, const char* nomArchivo);
void buscarDescripcion(char* descABuscar, const char* nomArchivo);
int crearFifo(const char* path, int modo);
void eliminarNewline(char* linea);
void eliminarNewlineN(char* linea);
int checkFile(const char* ruta);

const char* pathF1;
const char* pathF2;
const char* archivoArticulo;
int fifoRead;
int fifoWrite;

int main(int argc, const char *argv[])
{
    char  linea[TAM];
    Articulo art;
    Consulta consulta;

    if(argc == 1 || argc != 4 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "-help") == 0 || strcmp(argv[1], "-?") == 0)
    {
        ayuda();
        return 1;
    }
    else if (argc != 4)
    {
        puts("La ejecucion debe realizarse con 3 parametros. Consultar la ayuda con el parametro -h, -help o -?");
        exit(0);
    }

    signal(SIGINT,terminar);
    signal(SIGTERM, terminar);

    pathF1 = argv[1];
    pathF2 = argv[2];
    archivoArticulo = argv[3];

    if(checkFile(archivoArticulo) == 0)
    {
        printf("El directorio indicado no es un archivo regular.");
        exit(1);
    }

    crearDemonio();

        if((fifoRead = crearFifo(pathF1, O_RDONLY)) < 0)
            exit(fifoRead);

        if((fifoWrite = crearFifo(pathF2, O_WRONLY)) < 0)
            exit(fifoWrite);

        while (1)
        {
            while(read(fifoRead, linea, TAM) > 0)
            {
                consulta = identificarConsulta(linea);
                procesarConsulta(consulta, archivoArticulo);
            }
        }
    
    return 0;
}

int crearFifo(const char* path, int modo)
{
    int fr;
    mkfifo(path, 0660);
    fr = open(path, modo);

    if(fr < 0)
    {
        printf("Error al crear el fifo.\n");
        return -1;
    }
    return fr;
}

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

void procesarConsulta(Consulta consulta, const char* rutaArchivo)
{
    switch(consulta.tipoConsulta)
    {
        case 1:
            buscarID(consulta.valorConsulta, rutaArchivo);
            break;

        case 2:
            buscarDescripcion(consulta.valorConsulta, rutaArchivo);
            break;

        case 3:
            buscarProducto(consulta.valorConsulta, rutaArchivo);
            break;

        case 4:
            buscarMarca(consulta.valorConsulta, rutaArchivo);
            break;

        case 0:
            buscarMarca(consulta.valorConsulta, rutaArchivo);
    }
    return;
}


void buscarID(char* IDABuscar, const char* nomArchivo)
{
    Articulo art;
    int idSearch;
    char linea[200];
    char idString[10];
    FILE *fp = fopen(nomArchivo, "r") ;
    if (!fp)
        terminar(SIGINT);

    idSearch = atoi(IDABuscar);
    while (fscanf(fp,"%[^;];%[^;];%[^;];%[^\n]", idString, art.descripcion, art.producto, art.marca) && !feof(fp))
    {
        art.id = atoi(idString);
        if(art.id == idSearch)
        {
            crearLinea(art, linea);
            write(fifoWrite, linea, sizeof(linea));
            return;
        }
    }
    write(fifoWrite, "No se encontro coincidencia.\n",30);
    return;
}

void buscarDescripcion(char* descABuscar, const char* nomArchivo)
{
    Articulo art;
    char linea[200];
    char idString[10];
    FILE *fp = fopen(nomArchivo, "r") ;
    if (!fp)
        terminar(SIGINT);

    eliminarNewlineN(descABuscar);

    while (fscanf(fp,"%[^;];%[^;];%[^;];%[^\n]", idString, art.descripcion, art.producto, art.marca) && !feof(fp))
    {
        art.id = atoi(idString);
        if(strcmpi(descABuscar,art.descripcion) == 0)
        {
            crearLinea(art, linea);
            write(fifoWrite, linea, sizeof(linea));
            return;
        }
    }
    write(fifoWrite, "No se encontro coincidencia.\n",30);
    return;
}

void buscarProducto(char* producto, const char* nomArchivo)
{
    Articulo art;
    char linea[200];
    char idString[10];
    FILE *fp = fopen(nomArchivo, "r") ;
    if (!fp)
        terminar(SIGINT);

    eliminarNewlineN(producto);

    while (fscanf(fp,"%[^;];%[^;];%[^;];%[^\n]\n", idString, art.descripcion, art.producto, art.marca) && !feof(fp))
    {
        art.id = atoi(idString);
        if(strcmpi(producto,art.producto) == 0)
        {
            crearLinea(art, linea);
            write(fifoWrite, linea, sizeof(linea));
            return;
        }
    }
    write(fifoWrite, "No se encontro coincidencia.\n",30);
    return;
}

void buscarMarca(char* marcaBusqueda, const char* nomArchivo)
{
    Articulo art;
    char linea[200];
    char idString[10];
    FILE *fp = fopen(nomArchivo, "r") ;
    if (!fp)
        terminar(SIGINT);

    eliminarNewlineN(marcaBusqueda);

    while (fscanf(fp,"%[^;];%[^;];%[^;];%[^\n]", idString, art.descripcion, art.producto, art.marca) && !feof(fp))
    {
        eliminarNewline(art.marca);
        art.id = atoi(idString);
        if(strcmpi(marcaBusqueda,art.marca) == 0)
        {
            crearLinea(art, linea);
            write(fifoWrite, linea, sizeof(linea));
            return;
        }

    }
    write(fifoWrite, "No se encontro coincidencia.\n",30);
    return;
}

void crearLinea(Articulo art, char *linea)
{
    char id[8];
	itoa(art.id, id);
	printf(linea, "ID=%d Descripcion=%s Producto=%s Marca=%s\n", art.id, art.descripcion, art.producto, art.marca);
	sprintf(linea, "ID=%d Descripcion=%s Producto=%s Marca=%s\n", art.id, art.descripcion, art.producto, art.marca);
}

void intercambiar_cad(char *cad)
{
    char *inicio = cad;
	char *fin = cad;

	char *aux = cad;

	while((*fin))
		fin++;
	fin--;

    while (inicio < fin)
    {
    	*aux = *inicio;
		*inicio = *fin;
		*fin = *aux;
        inicio++;
        fin--;
    }
}

char* itoa(int num, char* cad)
{
    char *comienzo = cad;
    while (num != 0)
    {
        *cad = num % 10 + '0';
        num = num / 10;
		cad++;
    }
	*cad = '\0';
    intercambiar_cad(comienzo);
    return cad;
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
    *pFirstN = new_char;
}

void terminar(int signum)
{
    printf("\nSaliendo...\n");
    close(fifoRead);
    close(fifoWrite);
    unlink(pathF1);
    unlink(pathF2);
    exit(1);
}

void crearDemonio()
{ 
    pid_t pid;
    pid = fork();
    if (pid < 0)
    {
        exit(EXIT_FAILURE);
    }
    if (pid > 0)
    {
        exit(EXIT_SUCCESS);
    }
    if(pid == 0)
    {
        umask(0);

        if (setsid() < 0)
        {
            exit(EXIT_FAILURE);
        }

        /*if((chdir("/")) < 0)
        {
             exit(EXIT_FAILURE);
        }*/

        //close(STDIN_FILENO);
        //close(STDOUT_FILENO);
        //close(STDERR_FILENO);
    }
}

int checkFile(const char* ruta)
{
    struct stat path_stat;
    stat(ruta, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

void ayuda()
{
    puts("Ejercicio 3 - Trabajo Practico 3");
    puts("Contamos con un tabla que contiene la lista de artículos de la compañía en un txt. Campos: ID; Descr; Prod; Marca");
    puts("Se necesita hacer un proceso al que se pueda enviar una consulta desde otros procesos o comandos ejecutados en el sistema operativo y le devuelva los datos de los artículos; y para esto se decide realizar un demonio que lea de un FIFO las consultas y devuelva por otro FIFO los registros que encuentre.");
    puts("Parametros: ");
    puts("Primer parametro: Path Fifo Lectura");
    puts("Segundo parametro: Path Fifo Escritura");
    puts("Tercer parametro: path Archivo Base de Datos");

    puts("Ejemplo de Ejecucion: ./Ejercicio3 fifo.in fifo.out articulos.txt");
}

/** FIN DEL SCRIPT **/
