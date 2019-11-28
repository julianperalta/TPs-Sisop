#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

int main(int argc, const char *argv[])
{
    if(argc != 2)
    {
        printf("Debe enviar la ruta del fifo de escritura por parametro.\n");
        exit(0);
    }


    int fr;
    char bufferfifo[500];
    const char *path_salida = argv[1];
    mkfifo(path_salida, 0660);
    fr = open(path_salida, O_RDONLY);
    if (!fr)
    {
        printf("Error al abrir el fifo.\n");
        return -1;
    }
    while (read(fr, bufferfifo, 500) > 0)
    {
        printf("%s",bufferfifo);
    }
    return 0;
}