#!/bin/bash


#.SYNOPSIS
#Ejercicio n° 3 del trabajo practico n° 1 de Sistemas Operativos, Universidad Nacional de  La Matanza.
#.DESCRIPTION
#El script creará un Backup de un directorio pasado por parametro, cada 5 segundos. Para ello utilizara un demonio.

###################################################################################
###                             INICIO DE SCRIPT                                ###
#Nombre del script: ejercicio3.sh
#TP: Bash
#Número de ejercicio: 3
#Integrantes:
# Carbone, Emanuel - DNI: 40081161      
# De Stefano, Matías - DNI: 40130248 
# Fiorita, Leandro - DNI: 40012291
# Gentile, Soledad - DNI: 28053027 
# Peralta, Julián - DNI: 40242831
#Entrega: Segunda entrega
################################################################################



#FUNCIONES############################################

#Funcion errores

#Ayuda 

ayuda() {
	 echo "Ejercicio 3 - TP1 - Universidad Nacional de La Matanza 
	
	 Descripcion
	 El script ejecutara un demonio que realizara un backup de un directorio pasado por parametro cada un intervalo de tiempo. Cuenta la cantidad de archivos backup en el directorio y limpia el directorio donde se dejan los backups hasta una cierta cantidad
	
	 las opciones son:
	
	 [start]: Iniciar el demonio.
	 [stop]:  Finalizar el demonio.
	 [count]: Contar la cantidad de archivos backups que hay en el directorio.
	 [clear]: Limpiar el directorio de backups.
	 [play]:  Crear el backup en ese instante.
	
	 Modo de ejecucion:
	 ./ejercicio7.sh [start <dirBKP> <dirDest> <tiempo[s]>|stop|count|clear [cantidad]|play]
	
	 Donde:
	
	 dirBKP: es el directorio origen.
	 dirDest: es el directorio destino
	 tiempo[s]: intervalo de tiempo asignado.
	 cantidad: cantidad de archivos que quedaran en el backup."
	exit 0
}

#Verifico si hay un demonio activo

verificoDemonioActivo() {
	demonio=`ps -ef | grep ^$(whoami) | grep ./demonio.sh | grep -v grep | grep -v $$ | wc -l` #ps -ef ve los procesos completos
	if [ $demonio != 0 ]; then
	
		#demonio activo
		return 1 #Esta corriendo demonio
	else
		#No hay demonio activo
		return 0 #no esta corriendo demonio
	fi
}

iniciarDemonio() {
	 echo "Se iniciará el demonio"
		dirBKP="$(readlink -f $1)"
		dirDest="$(readlink -f $2)"
		nohup sh ./demonio.sh $dirBKP $dirDest $3 >/dev/null & #Corre el demonio, con el & lo deja corriendo en 2do plano
}	

######################################################


while [ -n $1 ]; do #Verifica que el string no sea nulo
	
	case $1 in
	
	start)
		verificoDemonioActivo
		if [ $? -eq 0 ]
			then
				iniciarDemonio $2 $3 $4
			else
				 echo "Ya hay un demonio corriendo actualmente"
		fi
		exit 1
		;;

	stop)
		verificoDemonioActivo
		if [ $? -eq 0 ]
			then
				echo "No hay un demonio corriendo actualmente"
			else
				pid=$(pgrep -x sh)
				kill $pid
				
		fi
		exit 1
		;;
		
	count)
		if [ -d "$2" ]
			then
				echo "Se ha encontrado la siguiente cantidad de backups en el directorio `$2`"
				ls -1q $2/bkp* | wc -l
			else
				echo "El directorio de backups no existe."
		fi
		exit 1
		;;
	
	clear)
		if [ -d "$2" ]
			then
				rm -rf $2 && mkdir $2
				echo "Se ha vaciado el directorio de backups"
			else
				echo "El directorio de backups no existe."
		fi
		exit 1
		;;
		
	play)
		bash ~/ejercicio3.sh start $2 $3 $4
		sleep $4
		bash ~/ejercicio3.sh stop $2 $3 $4
		exit 1
		;;
		
	*)
		echo "La opción ingresada es incorrecta."
		exit 1
		;;
	esac 
	shift
done


### Fin del Ejercicio