#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>

#define ERR_SOCK -1
#define ERR_CNX -1

typedef struct
{
    int id;
    char descripcion[100];
    char producto[50];
    char marca[50];
} Articulo;

struct sockaddr_in sa;

void setSockAdd(const char* dirIP, unsigned int puerto);

void imprimirResultados(Articulo art);

void ayuda();

void terminar(int signum);

void terminar_atExit();

int serverSockFD;

int main()
{
    int tamRespuestaServ;
    char buffer[256], bufferEnvio[256], respServ[16];
    Articulo artRespuesta;

    signal(SIGINT, terminar);
    signal(SIGTERM, terminar);
    atexit(terminar_atExit);

    // Creo el socket del servidor
    if((serverSockFD = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        return ERR_SOCK;
    }
    // Seteo la IP y el puerto a donde me voy a conectar
    setSockAdd("127.0.0.1", 15000);

    if(connect(serverSockFD, (struct sockaddr*) &sa, sizeof(sa)))
    {
        perror("connect");
        return ERR_CNX;
    }

    printf("Conectado al servidor...\n");
    printf("Para ver la ayuda escriba 'AYUDA'\n");
    bzero(buffer, 256);
    bzero(respServ, 16);
    strcpy(respServ, "Siguiente");

    // Mientras el cliente no escriba "QUIT" yo sigo leyendo de stdin
    while(strcmp(bufferEnvio, "QUIT") != 0)
    {
        bzero(bufferEnvio, 256);
        printf("\nIngrese su consulta: ");
        fgets(bufferEnvio, 256, stdin);

        if(strcmp(bufferEnvio, "AYUDA\n") == 0)
            ayuda();
        else
        {
            write(serverSockFD, bufferEnvio, sizeof(buffer));

            while((tamRespuestaServ = read(serverSockFD, buffer, 256)) == sizeof(Articulo))
            {
                memcpy(&artRespuesta, buffer, 256);
                imprimirResultados(artRespuesta);

                write(serverSockFD, respServ, 16);
            }

            if(tamRespuestaServ == 16 && strcmp(buffer, "Desconectar") == 0)
                break;
        }

        bzero(buffer, 256);
        fflush(stdin);
    }

    printf("Saliendo...\n");
    close(serverSockFD);
    printf("Sesion terminada\n");
    return 0;
}

void setSockAdd(const char* dirIP, unsigned int puerto)
{
    bzero(&(sa.sin_zero), 8);
    sa.sin_family = AF_INET;
    sa.sin_port = htons(puerto);
    sa.sin_addr.s_addr = inet_addr(dirIP);
}

void imprimirResultados(Articulo art)
{
    if(art.id == -1)
        puts("\nNo existe ningun articulo que coincida con su consulta\n");
    else
    {
        printf("\nID: %d\n", art.id);
        printf("Descripcion: ");
        puts(art.descripcion);
        printf("Producto: %s\n", art.producto);
        printf("Marca: ");
        puts(art.marca);
    }
    
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
    puts("Ejercicio 5 - Trabajo Práctico 3 - Segunda entrega\n");
    printf("\nIntegrantes:");
    printf("\n\tCarbone, Emanuel \t  40081161");
    printf("\n\tDe Stefano, Matias \t  40130248");
    printf("\n\tFiorita, Leandro \t  40012291");
    printf("\n\tGentile, Soledad \t  28053027");
    printf("\n\tPeralta, Julian \t  40242831\n\n");

    printf("Descripción:\n");
    printf("Se tiene una base de datos de articulos de un supermercado en un archivo de texto el cual se le pasa su dirección en el directorio actual del servidor.");
    puts("La identificación del articulo se hace a través de su número de ID. El archivo tiene el siguiente formato:\n");
    puts("\tID;Descripción;Producto;Marca\n");
    puts("Las consultas se pueden realizar para cualquier campo.");
    puts("El formato de las consultas es el siguiente:\n");
    puts("\tCampo=Valor - Ejemplo: Producto=HELADO\n");
    puts("* El servidor atiende las peticiones generadas desde un cliente. *");
    puts("* El ejercicio esta hecho con sockets y pthreads. *\n");

    puts("\n-------------------------\n");
}

void terminar(int signum)
{
    char respuesta[16];

    bzero(respuesta, 16);
    strcpy(respuesta, "QUIT\n");

    write(serverSockFD, respuesta, 16);
    read(serverSockFD, respuesta, 16);

    printf("Saliendo...\n");
    close(serverSockFD);
    exit(1);
}

void terminar_atExit()
{
    char respuesta[16];

    bzero(respuesta, 16);
    strcpy(respuesta, "QUIT\n");

    write(serverSockFD, respuesta, 16);
    read(serverSockFD, respuesta, 16);

    printf("Saliendo...\n");
    close(serverSockFD);
    exit(1);
}