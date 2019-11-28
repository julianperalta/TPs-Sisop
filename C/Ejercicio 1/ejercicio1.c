/*###################################################################################
###                               INICIO DE SCRIPT                                ###
#Nombre del script: ejercicio1.c
#TP: C
#Número de ejercicio: 1
#Integrantes:
# Carbone, Emanuel - DNI: 40081161      
# De Stefano, Matías - DNI: 40130248  
# Peralta, Julián - DNI: 40242831
#Entrega: Primera reentrega
###################################################################################*/

#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void error();
void getInfoProcess(pid_t, pid_t, char *, char *);
void getHelp();

pid_t main(pid_t argc, char *argv[]){

    //Validación de parámetros.
    if (argc >= 3){
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
    //Fin validación de parámetros.

    getInfoProcess(getpid(), getppid(), "Padre", "Normal");
    pid_t hijo1 = fork();

    if(hijo1 < 0){
        error(); return 1;
    }
    else if(hijo1 == 0){
        getInfoProcess(getpid(), getppid(), "Hijo", "Normal");
        pid_t nieto1 = fork();
        
        if(nieto1 < 0){
            error(); return 1;
        }
        else if(nieto1 == 0){        
            getInfoProcess(getpid(), getppid(), "Nieto", "Normal"); 
            pid_t bisnieto1 = fork();

            if(bisnieto1 < 0){
                error(); return 1;
            }
            else if(bisnieto1 == 0){
                getInfoProcess(getpid(), getppid(), "Bisnieto", "Normal");
                while(1) usleep(100); //Fin bisnieto1
            }
            else{
                pid_t bisnieto2 = fork();

                if(bisnieto2 < 0){
                    error(); return 1;
                }
                else if(bisnieto2 == 0){
                    getInfoProcess(getpid(), getppid(), "Bisnieto", "Normal");
                    while(1) usleep(10); //Fin bisnieto2
                }
                wait(NULL);
                wait(NULL); //Fin nieto1
            }
        }
        else{
            pid_t nieto2 = fork();
            
            if(nieto2 < 0){
                error(); return 1;
            }
            if(nieto2 == 0){
                getInfoProcess(getpid(), getppid(), "Nieto", "Normal");
                pid_t bisnieto3 = fork();
                
                if(bisnieto3 < 0){
                    error(); return 1;
                }
                else if(bisnieto3 == 0){
                    getInfoProcess(getpid(), getppid(), "Bisnieto", "Normal");
                    while(1) sleep(1); //Fin bisnieto3
                }
                else{
                    pid_t bisnieto4 = fork();

                    if(bisnieto4 < 0){
                        error();return 1;
                    }
                    else if(bisnieto4 == 0){
                        getInfoProcess(getpid(), getppid(), "Bisnieto", "Normal");
                        pid_t padreDemonio3 = fork();

                        if(padreDemonio3 < 0){
                            error(); return 1;
                        }
                        else if(padreDemonio3 == 0){
                            pid_t demonio3 = fork();

                            if(demonio3 < 0){
                                error(); return 1;
                            }
                            else if(demonio3 > 0){
                                exit(0);
                            }
                            else{
                                pid_t sid3 = setsid();
                                if(sid3 < 0) {
                                    return 1;
                                }
                                else{
                                    getInfoProcess(getpid(), getppid(), "Tátaranieto", "Demonio");
                                    while(1) usleep(10); //Fin demonio3
                                }
                            }
                        }
                        else{
                            wait(NULL);
                            while(1) usleep(10); //Fin bisnieto4
                        }
                    }
                    wait(NULL);
                    wait(NULL); //Fin nieto2
                }
            }
            wait(NULL);
            wait(NULL); //Fin hijo1
        }
    }
    else{
        pid_t hijo2 = fork();

        if(hijo2 < 0){
            error(); return 1;
        }
        else if(hijo2 == 0){
            getInfoProcess(getpid(), getppid(), "Hijo", "Normal");
            pid_t nieto3 = fork();

            if(nieto3 < 0){
                error(); return 1;
            }
            else if(nieto3 == 0){
                getInfoProcess(getpid(), getppid(), "Nieto", "Normal");
                pid_t bisnieto5 = fork();

                if(bisnieto5 < 0){
                    error(); return 1;
                }
                else if(bisnieto5 == 0){
                    getInfoProcess(getpid(), getppid(), "Bisnieto", "Normal");
                    pid_t padreDemonio1 = fork();
                    
                    if(padreDemonio1 < 0){
                        error(); return 1;
                    }
                    else if(padreDemonio1 == 0){
                        pid_t demonio1 = fork();

                        if(demonio1 < 0){
                            error(); return 1;
                        }
                        else if(demonio1 > 0){
                            exit(0);
                        }
                        else{
                            pid_t sid = setsid();
                            if(sid < 0) {
                                return 1;
                            }
                            else{
                                getInfoProcess(getpid(), getppid(), "Tátaranieto", "Demonio");
                                while(1) usleep(10); //Fin demonio1.
                            }
                        }
                    }
                    else{
                        pid_t padreDemonio2 = fork();

                        if(padreDemonio2 < 0){
                            error(); return 1;
                        }
                        else if(padreDemonio2 == 0){
                            pid_t demonio2 = fork();

                            if(demonio2 < 0){
                                error(); return 1;
                            }
                            else if(demonio2 > 0){
                                exit(0);
                            }
                            else{
                                pid_t sid2 = setsid();
                                if(sid2 < 0) {
                                    return 1;
                                }
                                else{
                                    getInfoProcess(getpid(), getppid(), "Tátaranieto", "Demonio");
                                    while(1) usleep(10); //Fin demonio2
                                }
                            }
                        }
                        else{
                            wait(NULL);
                            wait(NULL);
                            while(1) usleep(100); //Fin bisnieto5
                        }
                    }
                }
                else{
                    pid_t zombie2 = fork();
                    if(zombie2 < 0){
                        error(); return 1;
                    }
                    else if(zombie2 == 0){
                        getInfoProcess(getpid(), getppid(), "Bisnieto", "Zombie");
                        return 0;
                    }
                    else{
                        while(1) usleep(10); //Fin nieto3
                    }
                }
            }
            else{
                pid_t zombie1 = fork();
                if(zombie1 < 0){
                    error(); return 1;
                }
                else if(zombie1 == 0){
                    getInfoProcess(getpid(), getppid(), "Nieto", "Zombie");
                    return 1; //Fin zombie1
                }
                else{
                    while(1) usleep(10); //Fin hijo2                    
                }
            }
        }
        else{
            while(getchar() != '\n');
            exit(0);
        }
        //fin
    }
}

void getInfoProcess(pid_t pid, pid_t ppid, char *parentesco, char *tipo){
    printf("\nPID: %04d PPID: %04d Parentezco-Tipo: %s - %s\n\n", pid, ppid, parentesco, tipo);
}

void getHelp(){
    printf("\n\n-------------------------------------------------------------------------------------------\n");
    printf("\nNombre del ejecutable: ejercicio1");
    printf("\n-------------------------------------------------------------------------------------------\n");
    printf("\nDescripción:\n");
    printf("\n-El programa creará una familia de procesos, la cual está compuesta por:");
    printf("\n 2 hijos, 3 nietos, 5 bisnietos, 2 zombies y 3 demonios.");
    printf("\n\n-Por cada proceso, se indicará por pantalla el pid, el ppid, el parentesco y el tipo.");
    printf("\n-------------------------------------------------------------------------------------------\n");
    printf("\nParámetros: No posee.");
    printf("\n-------------------------------------------------------------------------------------------\n");
    printf("\nEjemplo de ejecución: ./ejercicio1");
    printf("\n-------------------------------------------------------------------------------------------\n\n");
}

void error(){
    printf("\nError al crear un proceso.\n\n");
}

/*###################################################################################
###                               FIN DE SCRIPT                                   ###
###################################################################################*/

