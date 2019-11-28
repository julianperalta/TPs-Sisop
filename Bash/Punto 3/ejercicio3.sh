#!/bin/bash

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
#Entrega: Tercera reentrega
###################################################################################

mostrarSintaxis()
{
	echo -e "\n-----------------------------------------------------------------------------------------------------------------------------------------------------"
	echo "Ejercicio 3 - TP Bash"
        echo -e "-----------------------------------------------------------------------------------------------------------------------------------------------------"
        echo "	-Descripción:
	 	El script ejecutará un demonio que realizará un backup de un directorio cada cierto intervalo de tiempo (o al instante). 
         	También puede indicarse que cuente la cantidad de archivos backup en el directorio o limpiar el mismo.
         
	 -Los comandos del demonio son:
	 	start: Iniciar el demonio.
         	stop:  Finalizar el demonio.
         	count: Contar la cantidad de archivos backups que hay en el directorio de destino.
         	clear: Limpiar el directorio de destino de backups.
         	play:  Crear el backup en ese instante.
		
		Para utilizar los comandos 'stop', 'count' y 'play', debe estar el demonio activo.

         -Sintaxis:
         	$0 start <dirBKP> <dirDest> <tiempo[s]> | stop | count | clear [cantidad] | play
        
         -Donde:
         	dirBKP: Directorio del que se hará el backup.
         	dirDest: Directorio destino del backup.
         	tiempo[s]: Intervalo de tiempo en segundos entre cada backup.
		cantidad: Archivos de backup que quedarán en el directorio de destino (los N más nuevos)."
	echo -e "-----------------------------------------------------------------------------------------------------------------------------------------------------\n"
}

esDirectorio(){
        if ! test -d "$1"; then                         # Si el parámetro no es un directorio, muestra
                echo -e "\n¡ERROR DE INVOCACIÓN!\n"     # el mensaje de error y se detiene la ejecución.
                echo -e "\"$1\" no es un directorio.\n"
                echo -e "Ejecute el script con -h, -? o --help para más información.\n"
                exit 1
        fi
}

errorInvocacion(){
        echo -e "\n¡ERROR DE INVOCACIÓN!\n" # Función genérica para mostrar mensajes de error.
        echo -e "$1.\n"
        echo -e "Ejecute el script con -h, -? o --help para más información.\n"
        exit 1
}

existeDemonio(){ # Devuelve el pid del demonio, en caso de que este exista.
	local pid
	pid=$(pgrep demonio.sh)
	echo $pid
}

if [ $# -eq 0 ] || [ $# -gt 4 ]; then # Verifico que la cantidad de parámetros sea válida (al menos, inicialmente).
	errorInvocacion "Cantidad de parámetros incorrecta"
fi

case "$1" in

	'start') 
		if [ $# -eq 4 ] && [ $4 -ge 1 ]; then # Cant. de parámetros correcta y tiempo mayor a 1?
			esDirectorio "$2" # Existe el directorio de bkp?
			esDirectorio "$3" # Existe el directorio destino?
			pidDemonio=`existeDemonio`
			
			if [ $pidDemonio ]; then # Verifico existencia demonio.
				echo -e "\nYa existe un demonio en ejecución."
			else
				echo -e "\nIniciando demonio..."
				echo $3 > "datos_demonio.txt" # Guardo path del directorio de almacenamiento.
				./demonio.sh "$2" "$3" $4 &
			        pidDemonio=$!
			fi

			echo -e "PID: $pidDemonio\n"
			exit 0
		else
			errorInvocacion "Cantidad de parámetros incorrecta o tiempo menor a 1"
		fi
	;;

	'stop')
		if [ $# -eq 1 ]; then # Cant. de parámetros correcta?
			pidDemonio=`existeDemonio`

			if [ $pidDemonio ]; then # Verifico existencia demonio.
				echo -e "\nDeteniendo demonio..."
				echo -e "PID: $pidDemonio"
				kill $pidDemonio # Ejecuto SIGTERM para el demonio.
				echo -e "Demonio detenido.\n"
			else
				echo -e "\nNo existe un demonio en ejecución.\n"
			fi
			exit 0
		else
			errorInvocacion "Cantidad de parámetros incorrecta"
		fi
	;;

	'count')
		if [ $# -eq 1 ]; then # Cant. de parámetros correcta?
			pidDemonio=`existeDemonio`

			if [ $pidDemonio ]; then # Verifico existencia demonio.
				kill -10 $pidDemonio # Envío señal al demonio (para realizar la cuenta de bkps).
			else
				echo -e "\nNo existe un demonio en ejecución.\n"
			fi
			exit 0
		else
			errorInvocacion "Cantidad de parámetros incorrecta"
		fi
	;;
	
	'clear')
		if [ $# -eq 2 ] && [ $2 -ge 0 ]; then # Cant. de parámetros correcta y cantidad restante válida?
			resto=$2
		elif [ $# -eq 1 ]; then # Cant. de parámetros correcta y cantidad restante = 0?
			resto=0
		else
			errorInvocacion "Cantidad de parámetros incorrecta o cantidad de archivos restantes menor a 0"
		fi

		dirStorage=`cat datos_demonio.txt` # Obtengo path del directorio de almacenamiento.
		archivosBKP=`ls -t "$dirStorage"` # Ordeno los archivos de bkp, de más nuevos a más viejos.
		IFS=$'\n'
		
		for archivo in $archivosBKP
		do
			if [ $resto -ne 0 ]; then # El archivo se conserva?
				resto=$((resto-1))
				continue
			else
				rm "$dirStorage$archivo" # Borro archivo
			fi
		done
		
		unset IFS
		exit 0
	;;
	
	'play')
		if [ $# -eq 1 ]; then # Cant. de parámetros correcta?
			pidDemonio=`existeDemonio`

			if [ $pidDemonio ]; then # Verifico existencia demonio.
				echo "Realizando backup instantáneo..."
				kill -12 $pidDemonio # Envío señal al demonio (para realizar el bkp instantáneo).
			else
				echo -e "\nNo existe un demonio en ejecución.\n"
			fi
			exit 0
		else
			errorInvocacion "Cantidad de parámetros incorrecta"
		fi
	;;

	'-?'|'-h'|'--help')
		if [ $# -eq 1 ]; then # Cant. de parámetros correcta?
			mostrarSintaxis # Muestro sintaxis
			exit 0
		else
			errorInvocacion "Cantidad de parámetros incorrecta"
		fi
	;;

	*) # Comando del demonio no válido
		errorInvocacion "Comando erróneo"
	;;
esac
					
