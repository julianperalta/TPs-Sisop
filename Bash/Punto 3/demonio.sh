#!/bin/bash

#FUNCIONES#################################################

#Principal
Iniciar_Demonio() {

#demonio crea los backups en el tiempo especificado
	while true 
	do
		sleep $1
		creaBackup
	done

}


#Crea un backup
creaBackup(){
	nombre_archivo=bkp_$(date '+%d_%m_%Y_%H_%M_%S').tar.gz
	tar czf "$DIRDEST"/$nombre_archivo "$DIRORIGEN"

}

######################################

#Por defecto va a empezar con un start

	#Guardamos los directorios 
	export DIRDEST="$2"
	export DIRORIGEN="$1"

	#capturo señal
	trap cuentaArchivos SIGUSR1
	trap eliminaArchivos SIGUSR2
	trap creaBackup SIGCONT


#Validacion de Parametros
if [ -d "$1" ] && [ -d "$2" ] #Veo que los primeros 2 parametros sean directorios existentes
then if [ "$3" -gt 0 ] #Veo que el tercer parámetro sea un número mayor a 0
	then
	Iniciar_Demonio "$3"  #Inicio el demonio mandando el tiempo para el sleep
	fi
else
	echo "No se ha ingresado alguno de los directorios necesarios"
fi
