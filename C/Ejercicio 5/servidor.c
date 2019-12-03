#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>

#define ERR_AP_SOCK -1
#define ERR_BIND_SOCK -1
#define ERR_CLI_SOCK -1

#define NO_MEM -1
#define EXITO 1

typedef struct
{
    int tipoConsulta;
    char valorConsulta[40];
    char consulta[60];
} Consulta;

typedef struct
{
    int id;
    char descripcion[100];
    char producto[50];
    char marca[50];
} Articulo;

/** Funciones servidor **/

typedef struct
{
    int socketCliFD;
    pthread_mutex_t mutex;
    char pathArchivo[500];
} t_args_aceptConex;

/*
* Abre un socket de servidor en la IP y el puerto que se le pasa por parametro.
* El socket queda listo para aceptar conexiones de clientes.
* Retorna el file descriptor del socket. En caso de error retorna -1
*/
int abrirSocket(uint16_t puerto);

/*
* Acepta la conexion de un cliente con el servidor.
* Retorna el file descriptor del socket cliente. En caso de error retorna -1
*/
int aceptarConexion(int servSockFD);

void* atenderCliente(void* args);

void buscarID(char* IDABuscar, char* nomArchivo, int clienteSockFD);

void buscarDescripcion(char* descABuscar, char* nomArchivo, int clienteSockFD);

void buscarProducto(char* producto, char* nomArchivo, int clienteSockFD);

void buscarMarca(char* marcaBusqueda, char* nomArchivo, int clienteSockFD);

void enviarArticuloVacio(int clienteSockFD);

void eliminarNewlineN(char* linea);

void eliminarNewline(char* linea);

int strcmpi(char const *a, char const *b);

Consulta identificarConsulta(char* consulta);

int checkDirectorio(const char* ruta);

void terminar(int signum);

void cancelarThreads();

void broadcast();

int cantPuntos(char* ip);

int digitoValido(char *octeto);

int validarIP(char* ip);

int validarPuerto(char* puerto);

void ayuda();

static void skeleton_daemon();

/** Fin funciones servidor **/

pthread_mutex_t mutex;
int socketServ;

int cantThreads = 0;
int cantClientes = 0;
int* clientes;
pthread_t* threadsClientes;

int main(int argc, char* argv[])
{
    int socketCli, contThreadActual = 0, resCrearThread;
    // Hay un error rarisimo si no uso este mutex auxiliar, probalo sin el auxiliar si te animas
    pthread_mutex_t auxMutex = PTHREAD_MUTEX_INITIALIZER;
    mutex = auxMutex;
    t_args_aceptConex argsAceptarConexion;

    signal(SIGINT,terminar);
    signal(SIGTERM, terminar);

    if(argc == 2 && (strcmp(argv[1],"-h")==0 || strcmp(argv[1], "-help")==0 || strcmp(argv[1], "-?")==0))
    {
        ayuda();
        return 1;
    }
    else if(argc != 3)
    {
        printf("Cantidad de parámetros incorrecta. Para consultar la ayuda ejecute el programa con los parámetros -h, -help o -?.");
        terminar(SIGINT);
    }

    if(checkDirectorio(argv[1]) == 0)
    {
        printf("El directorio indicado no es un archivo regular.");
        terminar(SIGINT);
    }

    // if(validarIP(argv[2]) == 0)
    // {
    //     // printf("La dirección IP indicada es inválida.");
    //     terminar(SIGINT);
    // }

    if(validarPuerto(argv[2]) == 0)
    {
        printf("El puerto indicado es inválido.");
        terminar(SIGINT);
    }

    skeleton_daemon();

    strncpy(argsAceptarConexion.pathArchivo, argv[1], 500); // Este es el parametro que tiene el nombre del archivo

    argsAceptarConexion.mutex = mutex;

    socketServ = abrirSocket(atoi(argv[2]));
    if(socketServ < 0)
        exit(EXIT_FAILURE);

    if(listen(socketServ, 10) < 0) // Hago que el socket empiece a escuchar clientes (10 clientes en espera)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    clientes = (int*)malloc(sizeof(int));
    threadsClientes = (pthread_t*)malloc(sizeof(pthread_t));

    while(1)
    {
        socketCli = aceptarConexion(socketServ);

        if(socketCli > 0)
        {
            argsAceptarConexion.socketCliFD = socketCli;

            resCrearThread = pthread_create(&threadsClientes[cantThreads], NULL, atenderCliente, &argsAceptarConexion);

            if(resCrearThread < 0)
            {
                // printf("\nError de creacion de thread de atencion a cliente.\n");
                // printf("Cerrando file descriptor %d...\n", socketCli);
                close(socketCli);
            }
            else
            {
                clientes[cantClientes] = socketCli;

                clientes = (int*)realloc(clientes, sizeof(int) * (cantClientes + 1));
                threadsClientes = (pthread_t*)realloc(threadsClientes, sizeof(pthread_t) * (cantThreads + 1));

                cantThreads++;
                cantClientes++;
                // printf("\nCreado el thread para el cliente %d", socketCli);
            }
        }
        else
            printf("Error de conexion con el cliente.\n");
    }

    terminar(SIGINT); // Por las dudas
    return 0;
}

int abrirSocket(uint16_t puerto)
{
    int servSockFD = 0;
    struct sockaddr_in nombreSocket;

    /* Creo el socket del servidor */
    servSockFD = socket(PF_INET,SOCK_STREAM,0);
    if(servSockFD < 0)
    {
        perror("socket");
        return ERR_AP_SOCK;
    }

    /* Le doy un nombre al socket */
    nombreSocket.sin_family = AF_INET;    // La familia de direcciones que puede leer son IPv4
    nombreSocket.sin_port = htons(puerto);
    nombreSocket.sin_addr.s_addr = htonl(INADDR_ANY); // Que acepte cualquier direccion

    if(bind(servSockFD, (struct sockaddr*) &nombreSocket, sizeof(nombreSocket)) < 0)
    {
        perror("bind");
        close(servSockFD); // En caso de error cierro el file descriptor del socket
        return ERR_BIND_SOCK;
    }

    return servSockFD;
}

int aceptarConexion(int servSockFD)
{
    int clienteSockFD;
    struct sockaddr cliente;
    socklen_t longCliente;

    longCliente = sizeof(cliente);

    clienteSockFD = accept(servSockFD, &cliente, &longCliente);

    if(clienteSockFD < 0)
    {
        perror("cliente sock");
        return ERR_CLI_SOCK;
    }

    // printf("Cliente aceptado\n");
    return clienteSockFD;
}

void* atenderCliente(void* args)
{
    t_args_aceptConex argumentos = *(t_args_aceptConex*)args;
    pthread_mutex_t mutex = argumentos.mutex;
    int clienteSockFD = argumentos.socketCliFD;
    ssize_t resLectura;
    char datos[256], pathArchivoArt[500], respuesta[16];
    Consulta consulta;

    bzero(respuesta, 16);
    bzero(datos, 256);
    bzero(pathArchivoArt, 500);
    strcpy(pathArchivoArt, argumentos.pathArchivo);
    strcpy(respuesta, "Desconectar");

    // printf("\nAtendiendo cliente...\n");
    do
    {
        resLectura = read(clienteSockFD, datos, 256);

        if(resLectura < 0)
            printf("Error de lectura");
        else
        {

            consulta = identificarConsulta(datos);
            
            // Inicio de zona critica
            pthread_mutex_lock(&mutex);

            // if(strncmp("ID", datos, 2) == 0)
            //     buscarID(datos, pathArchivoArt, clienteSockFD);
            // else if(strncmp("ARTICULO", datos, 8) == 0)
            //     buscarDescripcion(datos, pathArchivoArt, clienteSockFD);
            // else if(strncmp("MARCA", datos, 5) == 0)
            //     buscarMarca(datos, pathArchivoArt, clienteSockFD);
            // else if(strncmp("PRODUCTO", datos, 8) == 0)
            //     buscarProducto(datos, pathArchivoArt, clienteSockFD);
            // else
            //     enviarArticuloVacio(clienteSockFD);

            switch(consulta.tipoConsulta)
            {
                case 1:
                    buscarID(consulta.valorConsulta, pathArchivoArt, clienteSockFD);
                    break;

                case 2:
                    buscarDescripcion(consulta.valorConsulta, pathArchivoArt, clienteSockFD);
                    break;

                case 3:
                    buscarProducto(consulta.valorConsulta, pathArchivoArt, clienteSockFD);
                    break;

                case 4:
                    buscarMarca(consulta.valorConsulta, pathArchivoArt, clienteSockFD);
                    break;

                case 5:
                    break;
                case 0:
                    buscarMarca(consulta.valorConsulta, pathArchivoArt, clienteSockFD);
                    // //printf("Se ha ingresado una consulta invalida.\n");
                    //terminar(SIGINT);
            }

            pthread_mutex_unlock(&mutex);
            // Fin de zona critica
        }

    } while(consulta.tipoConsulta != 5);

    write(clienteSockFD, respuesta, 16);
    close(clienteSockFD);
    //pthread_exit(NULL);
    return NULL;
}

void buscarID(char* IDABuscar, char* nomArchivo, int clienteSockFD)
{
    Articulo art;
    char artEncontrado = 'n';

    int idSearch;
    char idString[10], respuesta[16];
    FILE *fp = fopen(nomArchivo, "r") ;
    if (!fp)
    {
        // printf("No existe el archivo.\n");
        terminar(SIGINT);
        return;
    }
    bzero(respuesta, 16);

    eliminarNewlineN(IDABuscar);
    idSearch = atoi(IDABuscar);
    fseek(fp,1L, SEEK_SET);
    while (fscanf(fp,"%[^;];%[^;];%[^;];%[^\n]", idString, art.descripcion, art.producto, art.marca) && !feof(fp))
    {
        art.id = atoi(idString);
        if(art.id == idSearch)
        {
            artEncontrado = 's';
            // Como es un ID solo hay una coincidencia
            write(clienteSockFD, &art, sizeof(Articulo));
            read(clienteSockFD, respuesta, 16);
            break;
        }
    }

    if(artEncontrado == 'n')
    {
        art.id = 0;
        write(clienteSockFD, &art, sizeof(Articulo));
        read(clienteSockFD, respuesta, 16);
        bzero(respuesta, 16);
    }

    // Aviso al cliente que termino la busqueda
    strcpy(respuesta, "Terminado");
    write(clienteSockFD, respuesta, 16);
    fclose(fp);
    return;
}

void buscarDescripcion(char* descABuscar, char* nomArchivo, int clienteSockFD)
{
    Articulo art;
    char artEncontrado = 'n';

    char idString[10], respuesta[16];
    FILE *fp = fopen(nomArchivo, "r") ;
    if (!fp)
    {
        // printf("No existe el archivo.\n");
        terminar(SIGINT);
        return;
    }
    bzero(respuesta, 16);

    eliminarNewlineN(descABuscar);

    fseek(fp,1L, SEEK_SET);
    while (fscanf(fp,"%[^;];%[^;];%[^;];%[^\n]", idString, art.descripcion, art.producto, art.marca) && !feof(fp))
    {
        art.id = atoi(idString);
        if(strcmpi(descABuscar, art.descripcion) == 0)
        {
            artEncontrado = 's';
            write(clienteSockFD, &art, sizeof(Articulo));

            // Leo la respuesta del cliente que me dice "siguiente"
            // Cuando la lei sigo escribiendo las coincidencias
            read(clienteSockFD, respuesta, 16);
        }
    }
    bzero(respuesta, 16);

    if(artEncontrado == 'n')
    {
        art.id = 0;
        write(clienteSockFD, &art, sizeof(Articulo));
        read(clienteSockFD, respuesta, 16);
        bzero(respuesta, 16);
    }

    strcpy(respuesta, "Terminado");
    write(clienteSockFD, respuesta, 16);
    fclose(fp);
    return;
}

void buscarProducto(char* producto, char* nomArchivo, int clienteSockFD)
{
    Articulo art;
    char artEncontrado = 'n';

    char idString[10], respuesta[16];
    FILE *fp = fopen(nomArchivo, "r") ;
    if (!fp)
    {
        // printf("No existe el archivo.\n");
        terminar(SIGINT);
        return;
    }
    bzero(respuesta, 16);

    eliminarNewlineN(producto);

    fseek(fp,1L, SEEK_SET);
    while (fscanf(fp,"%[^;];%[^;];%[^;];%[^\n]\n", idString, art.descripcion, art.producto, art.marca) && !feof(fp))
    {
        art.id = atoi(idString);
        if(strcmpi(producto, art.producto) == 0)
        {
            artEncontrado = 's';
            write(clienteSockFD, &art, sizeof(Articulo));

            // Leo la respuesta del cliente que me dice "siguiente"
            // Cuando la lei sigo escribiendo las coincidencias
            read(clienteSockFD, respuesta, 16);
        }
    }
    bzero(respuesta, 16);

    if(artEncontrado == 'n')
    {
        art.id = 0;
        write(clienteSockFD, &art, sizeof(Articulo));
        read(clienteSockFD, respuesta, 16);
        bzero(respuesta, 16);

    }
    strcpy(respuesta, "Terminado");
    write(clienteSockFD, respuesta, 16);
    fclose(fp);
    return;
}

void buscarMarca(char* marcaBusqueda, char* nomArchivo, int clienteSockFD)
{
    Articulo art;
    char artEncontrado = 'n';

    char idString[10], respuesta[16];
    FILE *fp = fopen(nomArchivo, "r") ;
    if (!fp)
    {
        // printf("No existe el archivo.\n");
        terminar(SIGINT);
        return;
    }
    bzero(respuesta, 16);

    eliminarNewlineN(marcaBusqueda);

    fseek(fp,1L, SEEK_SET);
    while (fscanf(fp,"%[^;];%[^;];%[^;];%[^\n]", idString, art.descripcion, art.producto, art.marca) && !feof(fp))
    {
        eliminarNewline(art.marca);
        art.id = atoi(idString);
        if(strcmpi(marcaBusqueda, art.marca) == 0)
        {
            artEncontrado = 's';
            write(clienteSockFD, &art, sizeof(Articulo));

            // Leo la respuesta del cliente que me dice "siguiente"
            // Cuando la lei sigo escribiendo las coincidencias
            read(clienteSockFD, respuesta, 16);
        }
    }

    bzero(respuesta, 16);

    if(artEncontrado == 'n')
    {
        art.id = 0;
        write(clienteSockFD, &art, sizeof(Articulo));
        read(clienteSockFD, respuesta, 16);
        bzero(respuesta, 16);
    }

    strcpy(respuesta, "Terminado");
    write(clienteSockFD, respuesta, 16);
    fclose(fp);
    return;
}

void enviarArticuloVacio(int clienteSockFD)
{
    Articulo art;
    char respuesta[16];

    bzero(respuesta, 16);

    art.id = -1;
    bzero(art.descripcion, 100);
    bzero(art.marca, 50);
    bzero(art.producto, 50);

    write(clienteSockFD, &art, sizeof(Articulo));
    read(clienteSockFD, respuesta, 16);
    strcpy(respuesta, "Terminado");
    write(clienteSockFD, respuesta, 16);
}

void eliminarNewlineN(char* linea)
{
    char new_char = '\0';
    char* pFirstN = strstr(linea, "\n");
    if(pFirstN)
        *pFirstN = new_char;
}

void eliminarNewline(char* linea)
{
    char new_char = '\0';
    char* pFirstN = strstr(linea, "\r");
    if(pFirstN)
        *pFirstN = new_char;
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
        eliminarNewlineN(consulta);
        if(strcmpi(consulta, "quit") == 0)
            retorno.tipoConsulta = 5;
        else
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

int checkDirectorio(const char* ruta)
{
    struct stat path_stat;
    stat(ruta, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

void terminar(int signum)
{
    printf("\nSaliendo...\n");
    broadcast();

    close(socketServ);
    pthread_mutex_destroy(&mutex);
    exit(1);
}

void cancelarThreads()
{
    int i;
    void *resJoin;

    for(i = 0; i < cantThreads; i++)
    {
        pthread_cancel(threadsClientes[i]);
        pthread_join(threadsClientes[i], &resJoin);

        if(resJoin == PTHREAD_CANCELED)
            printf("Thread %d cancelado\n", i);
    }
}

void broadcast()
{
	char mensaje[16]="Cancelado\0";
	for(int i=0; i<cantClientes; i++)
	{
		printf("<< Broadcast a cliente: <%d> || size: <%ld> >>\n", clientes[i], sizeof(mensaje));
		write(clientes[i], &mensaje, 16);
        close(clientes[i]);
	}
    cancelarThreads();
    free(threadsClientes);
	free(clientes);
}

int cantPuntos(char* ip)
{
    int cantPuntos = 0;

    while(*ip)
    {
        if(*ip == '.')
            cantPuntos++;
        ip++;
    }

    return cantPuntos;
}

int digitoValido(char *octeto)
{ 
    while (*octeto) { 
        if (*octeto >= '0' && *octeto <= '9') 
            ++octeto; 
        else
            return 0; 
    } 
    return 1; 
} 

int validarIP(char* ip)
{
    char copiaIP[30], *token;
    int contPuntos = 0, octeto;
    
    if(ip == NULL)
        return 0;
    
    eliminarNewlineN(ip);
    strcpy(copiaIP, ip);
    
    if(cantPuntos(copiaIP) != 3)
        return 0;

    token = strtok(copiaIP, ".");

    if(token == NULL)
        return 0;

    while(token != NULL)
    {
        if(!digitoValido(token))
            return 0;

        octeto = atoi(token);

        if(octeto >= 0 && octeto <= 255)
        {
            token = strtok(NULL, ".");
            if(token != NULL)
                contPuntos++;
        }
        else
            return 0;
    }

    if(contPuntos != 3)
        return 0;
    return 1;
}

int validarPuerto(char* puerto)
{
    char copiaPuerto[10];
    int nroPuerto;

    if(puerto == NULL)
        return 0;
    eliminarNewlineN(puerto);

    strcpy(copiaPuerto, puerto);

    if(!digitoValido(copiaPuerto))
        return 0;

    nroPuerto = atoi(copiaPuerto);

    if(nroPuerto < 10000 || nroPuerto > 20000)
        return 0;
    return 1;
}

void ayuda()
{
    // puts("-------------------------\n");
    // puts("El programa se conecta al servidor para hacer consultas sobre un archivo de articulos varios");
    // puts("Las consultas posibles son:\n");
    // puts("\t* ID=<Id_articulo>");
    // puts("\t* ARTICULO=<Descripcion_articulo>");
    // puts("\t* PRODUCTO=<Nombre_articulo>");
    // puts("\t* MARCA=<Marca_articulo>");
    // puts("\n-------------------------\n");

    puts("\n-------------------------\n");

    puts("Ejercicio 5 - Trabajo Práctico 3 - Tercera entrega\n");
    printf("\nIntegrantes:");
    printf("\n\tCarbone, Emanuel \t  40081161");
    printf("\n\tDe Stefano, Matias \t  40130248");
    printf("\n\tFiorita, Leandro \t  40012291");
    printf("\n\tGentile, Soledad \t  28053027");
    printf("\n\tPeralta, Julian \t  40242831\n\n");

    printf("Descripción:\n");
    printf("Se tiene una base de datos de artículos de un supermercado en un archivo de texto el cual se le pasa su dirección en el directorio actual del servidor.");
    puts("La identificación del articulo se hace a través de su número de ID. El archivo tiene el siguiente formato:\n");
    puts("\tID;Descripción;Producto;Marca\n");
    puts("Las consultas se pueden realizar para cualquier campo.");
    puts("El formato de las consultas es el siguiente:\n");
    puts("\tCampo=Valor - Ejemplo: Producto=HELADO\n");
    puts("* El servidor atiende las peticiones generadas desde un cliente. *");
    puts("* El ejercicio esta hecho con sockets y pthreads. *\n");

    puts("Los parámetros a pasar al programa son los siguientes:");
    puts("\t* Path al archivo de artículos");
    // puts("\t* Dirección IP del servidor");
    puts("\t* Número del puerto del servidor (entre 10000 y 20000 incluídos)\n");

    puts("Ejemplo de ejecución de este programa:");
    puts("\t./server /ruta/archivo/articulos.txt 127.0.0.1 15000");

    puts("\n-------------------------\n");
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


