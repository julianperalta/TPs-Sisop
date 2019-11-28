#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>

#define ERR_AP_SOCK -1
#define ERR_BIND_SOCK -1
#define ERR_CLI_SOCK -1

#define NO_MEM -1
#define EXITO 1

#define MAX_THREADS 1000

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
* Abre un socket de servidor en el puerto que se le pasa por parametro.
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

int checkDirectorio(const char* ruta);

void terminar(int signum);

static void skeleton_daemon();

/** Fin funciones servidor **/

pthread_mutex_t mutex;
int socketServ;

int main(int argc, char* argv[])
{
    int socketCli, contThreadActual = 0, resCrearThread;
    pthread_t threadCli[MAX_THREADS];
    // Hay un error rarisimo si no uso este mutex auxiliar, probalo sin el auxiliar si te animas
    pthread_mutex_t auxMutex = PTHREAD_MUTEX_INITIALIZER;
    mutex = auxMutex;
    t_args_aceptConex argsAceptarConexion;

    skeleton_daemon();
    signal(SIGINT,terminar);
    signal(SIGTERM, terminar);

    if(argc != 2)
    {
        printf("No se paso ningun path para el archivo.");
        terminar(SIGINT);
    }

    if(checkDirectorio(argv[1]) == 0)
    {
        printf("El directorio indicado no es un archivo regular.");
        terminar(SIGINT);
    }

    strncpy(argsAceptarConexion.pathArchivo, argv[1], 500); // Este es el parametro que tiene el nombre del archivo

    argsAceptarConexion.mutex = mutex;

    socketServ = abrirSocket(15000); // Lo abro en el puerto 15000 (porque si, se me ocurrio)
    if(socketServ < 0)
        exit(EXIT_FAILURE);

    if(listen(socketServ, 5) < 0) // Hago que el socket empiece a escuchar clientes (5 clientes en espera)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        socketCli = aceptarConexion(socketServ);

        if(socketCli > 0)
        {
            argsAceptarConexion.socketCliFD = socketCli;

            if(contThreadActual < MAX_THREADS)
            {
                resCrearThread = pthread_create(&threadCli[contThreadActual], NULL, atenderCliente, &argsAceptarConexion);

                if(resCrearThread < 0)
                {
                    printf("Error de creacion de thread de atencion a cliente.\n");
                    printf("Cerrando file descriptor %d...\n", socketCli);
                    close(socketCli);
                }
                else
                {
                    contThreadActual++;
                    printf("Creado el thread para el cliente %d", socketCli);
                    pthread_detach(threadCli[contThreadActual]);
                }
            }
            else
            {
                printf("No hay mas threads para crear, reinicie el servidor\n");
                close(socketCli);
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
    nombreSocket.sin_addr.s_addr = htons(INADDR_ANY); // Que acepte cualquier direccion

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

    printf("Cliente aceptado\n");
    return clienteSockFD;
}

void* atenderCliente(void* args)
{
    t_args_aceptConex argumentos = *(t_args_aceptConex*)args;
    pthread_mutex_t mutex = argumentos.mutex;
    int clienteSockFD = argumentos.socketCliFD;
    ssize_t resLectura;
    char datos[256], pathArchivoArt[500], respuesta[16];

    bzero(respuesta, 16);
    bzero(datos, 256);
    bzero(pathArchivoArt, 500);
    strcpy(pathArchivoArt, argumentos.pathArchivo);
    strcpy(respuesta, "Desconectar");

    printf("Atendiendo cliente...");
    do
    {
        resLectura = read(clienteSockFD, datos, 256);

        if(resLectura < 0)
            printf("Error de lectura");
        else if(strcmp(datos, "QUIT\n") != 0)
        {
            // Inicio de zona critica
            pthread_mutex_lock(&mutex);

            if(strncmp("ID", datos, 2) == 0)
                buscarID(datos, pathArchivoArt, clienteSockFD);
            else if(strncmp("ARTICULO", datos, 8) == 0)
                buscarDescripcion(datos, pathArchivoArt, clienteSockFD);
            else if(strncmp("MARCA", datos, 5) == 0)
                buscarMarca(datos, pathArchivoArt, clienteSockFD);
            else if(strncmp("PRODUCTO", datos, 8) == 0)
                buscarProducto(datos, pathArchivoArt, clienteSockFD);
            else
                enviarArticuloVacio(clienteSockFD);

            pthread_mutex_unlock(&mutex);
            // Fin de zona critica
        }

    } while(strcmp(datos, "QUIT\n") != 0);

    write(clienteSockFD, respuesta, 16);
    close(clienteSockFD);
    //pthread_exit(NULL);
    return NULL;
}

void buscarID(char* IDABuscar, char* nomArchivo, int clienteSockFD)
{
    Articulo art;

    int idSearch;
    char idString[10], respuesta[16];
    FILE *fp = fopen(nomArchivo, "r") ;
    if (!fp)
    {
        printf("No existe el archivo.\n");
        return;
    }
    bzero(respuesta, 16);

    idSearch = atoi(IDABuscar + 3); // Le paso la parte del string que viene despues del '='

    fseek(fp,1L, SEEK_SET);
    while (fscanf(fp,"%[^;];%[^;];%[^;];%[^\n]", idString, art.descripcion, art.producto, art.marca) && !feof(fp))
    {
        art.id = atoi(idString);
        if(art.id == idSearch)
        {
            // Como es un ID solo hay una coincidencia
            write(clienteSockFD, &art, sizeof(Articulo));
            break;
        }
    }

    // Aviso al cliente que termino la busqueda
    strcpy(respuesta, "Terminado");
    write(clienteSockFD, respuesta, 16);
    fclose(fp);
    return;
}

void buscarDescripcion(char* desc, char* nomArchivo, int clienteSockFD)
{
    Articulo art;

    char idString[10], respuesta[16], descABuscar[100];
    FILE *fp = fopen(nomArchivo, "r") ;
    if (!fp)
    {
        printf("No existe el archivo.\n");
        return;
    }
    bzero(respuesta, 16);
    bzero(descABuscar, 100);

    eliminarNewlineN(desc);
    strcpy(descABuscar, desc + 12); // Le guardo el nombre del producto

    fseek(fp,1L, SEEK_SET);
    while (fscanf(fp,"%[^;];%[^;];%[^;];%[^\n]", idString, art.descripcion, art.producto, art.marca) && !feof(fp))
    {
        art.id = atoi(idString);
        if(strcmpi(descABuscar,art.descripcion) == 0)
        {
            write(clienteSockFD, &art, sizeof(Articulo));

            // Leo la respuesta del cliente que me dice "siguiente"
            // Cuando la lei sigo escribiendo las coincidencias
            read(clienteSockFD, respuesta, 16);
        }
    }
    bzero(respuesta, 16);

    strcpy(respuesta, "Terminado");
    write(clienteSockFD, respuesta, 16);
    fclose(fp);
    return;
}

void buscarProducto(char* producto, char* nomArchivo, int clienteSockFD)
{
    Articulo art;

    char idString[10], respuesta[16], productoBusqueda[50];
    FILE *fp = fopen(nomArchivo, "r") ;
    if (!fp)
    {
        printf("No existe el archivo.\n");
        return;
    }
    bzero(respuesta, 16);
    bzero(productoBusqueda, 50);

    eliminarNewlineN(producto);
    strcpy(productoBusqueda, producto + 9); // Le guardo el nombre del producto

    fseek(fp,1L, SEEK_SET);
    while (fscanf(fp,"%[^;];%[^;];%[^;];%[^\n]\n", idString, art.descripcion, art.producto, art.marca) && !feof(fp))
    {
        art.id = atoi(idString);
        if(strcmpi(producto,art.producto) == 0)
        {
            write(clienteSockFD, &art, sizeof(Articulo));

            // Leo la respuesta del cliente que me dice "siguiente"
            // Cuando la lei sigo escribiendo las coincidencias
            read(clienteSockFD, respuesta, 16);
        }
    }
    bzero(respuesta, 16);

    strcpy(respuesta, "Terminado");
    write(clienteSockFD, respuesta, 16);
    fclose(fp);
    return;
}

void buscarMarca(char* marca, char* nomArchivo, int clienteSockFD)
{
    Articulo art;

    char idString[10], respuesta[16], marcaBusqueda[50];
    FILE *fp = fopen(nomArchivo, "r") ;
    if (!fp)
    {
        printf("No existe el archivo.\n");
        return;
    }
    bzero(respuesta, 16);
    bzero(marcaBusqueda, 50);

    eliminarNewlineN(marca);
    strcpy(marcaBusqueda, marca + 6); // Le guardo el nombre de la marca

    fseek(fp,1L, SEEK_SET);
    while (fscanf(fp,"%[^;];%[^;];%[^;];%[^\n]", idString, art.descripcion, art.producto, art.marca) && !feof(fp))
    {
        eliminarNewline(art.marca);
        art.id = atoi(idString);
        if(strcmpi(marcaBusqueda,art.marca) == 0)
        {
            write(clienteSockFD, &art, sizeof(Articulo));

            // Leo la respuesta del cliente que me dice "siguiente"
            // Cuando la lei sigo escribiendo las coincidencias
            read(clienteSockFD, respuesta, 16);
        }
    }

    bzero(respuesta, 16);

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

int checkDirectorio(const char* ruta)
{
    struct stat path_stat;
    stat(ruta, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

void terminar(int signum)
{
    printf("\nSaliendo...\n");
    close(socketServ);
    pthread_mutex_destroy(&mutex);
    exit(1);
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


