#!/bin/bash

trap contarBKPs SIGUSR1 # Capturo señal para contar bkps.
trap crearBKP SIGUSR2 # Captura señal para realizar backup instantáneo.

contarBKPs()
{
        cantidad=$(ls "$dirDest" | wc -l) # Obtengo cantidad de archivos del directorio de almacenamiento.
        echo -e "\nCantidad de archivos de backup: $cantidad"
}

crearBKP()
{	
	nombreDirectorio=$(basename "$dirBKP") # Obtengo nombre base del directorio del que se realiza bkp.
	fechaHora=`date +%F_%T` # Obtengo fecha y hora actual.
	tar cf "$dirDest$nombreDirectorio""_$fechaHora.tar" "$dirBKP" > /dev/null # Realizo backup.
}

# Reasigno variables para que sean utilizadas a la hora de la llegada de señales.
dirBKP=$1
dirDest=$2
intervalo=$3

while true # El demonio se encuentra ejecutando cada "X" tiempo.
do
	tiempo=$intervalo
	crearBKP # Creo un backup

	while [ $tiempo -gt 0 ] # Va haciendo una espera de a un segundo, ya que si hago todo el sleep
	do			# en una sola instancia, no me tomaría el comando "play" a tiempo.
		sleep 1
		tiempo=$((tiempo-1))
	done
done
