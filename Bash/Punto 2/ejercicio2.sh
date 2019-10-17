#!/bin/bash

###################################################################################
###                             INICIO DE SCRIPT                                ###
#Nombre del script: ejercicio2.sh
#TP: Bash
#Número de ejercicio: 2
#Integrantes:
# Carbone, Emanuel - DNI: 40081161      
# De Stefano, Matías - DNI: 40130248 
# Fiorita, Leandro - DNI: 40012291
# Gentile, Soledad - DNI: 28053027 
# Peralta, Julián - DNI: 40242831
#Entrega: Primera entrega
###################################################################################

mostrarSintaxis(){
        echo -e "\n--------------------------------------------------------------"
        echo "Ejecución del script: $0 [OPCION] [DIRECTORIO]"
        echo "--------------------------------------------------------------"
        echo "OPCION:"
        echo "-h, -?, --help: Muestra ayuda."
        echo "-r: Ejecución recursiva."
        echo "--------------------------------------------------------------"
        echo "DIRECTORIO: Si no se indica uno, se toma el directorio actual."
        echo -e "--------------------------------------------------------------\n"
}

esDirectorio(){
        if ! test -d "$1"; then				# Si el parámetro no es un directorio, muestra
                echo -e "\n¡ERROR DE INVOCACIÓN!\n" 	# el mensaje de error y se detiene la ejecución.
                echo -e "\"$1\" no es un directorio.\n"
		echo -e "Ejecute el script con -h, -? o --help para más información.\n"
                unset IFS
		exit -1
        fi
}

procesarArchivosREC(){

	directorios=($(find "$1" -type d)) # Obtengo la lista de todos los directorios
					   # a partir del directorio raíz.
 	for dir in "${directorios[@]}"; do # Por cada directorio, proceso los archivos.
		procesarArchivos "$dir"
 	done
}

procesarArchivos(){

	archivos=($(find "$1" -maxdepth 1 -type f -printf "%f\n" | grep "\s")) # Obtengo la lista de archivos con 
									       # espacios del directorio actual.
	
	renombrarArchivos "$1" "${archivos[@]}" # Envío el directorio y la lista de archivos.
}

renombrarArchivos(){
	
	listaArchivos=("$@") # Obtengo el directorio (primer parámetro) y la lista de archivos. 
	cantidad=$((${#listaArchivos[@]}-1))
	
	for i in `seq 1 $cantidad`; do # Por cada elemento de la lista, renombro.
	
	new_name=`echo ${listaArchivos[i]} | sed 's/\s\+/_/g'` # Obtengo el nuevo nombre.
	mv -i "$1/${listaArchivos[i]}" "$1/$new_name"	       # Renombro. Si el archivo renombrado existe,
 							       # consulta si sobrescribir o no.
	if ! test -e "$1/${listaArchivos[i]}"; then
  		contador=$((contador+1))		       # Si el archivo de nombre con espacios no existe,
	fi						       # es porque fue renombrado. Incremento.
	
	done
}

contador=0
IFS=$'\n'

if [ $# == 0 ]; then	#Si no hay parámetros, el directorio es el actual y no hay recursividad.
	procesarArchivos "./"
	echo -e "\nEjecución finalizada:\n"
	echo -e "Se renombraron $contador archivos.\n"
	unset IFS
	exit 0
fi

if [ $# == 1 ]; then	#Si hay un solo parámetro:
	if [ "$1" == '-h' ] || [ "$1" == '-?' ] || [ "$1" == '--help' ]; then # Solicitó ayuda?
		mostrarSintaxis
		exit 0
	elif [ "$1" == '-r' ]; then # Ejecuta recursivamente?
		procesarArchivosREC "./"
        	echo -e "\nEjecución finalizada:\n"
        	echo -e "Se renombraron $contador archivos.\n"
        	unset IFS
		exit 0	
	else
		esDirectorio "$1" # No hay recursividad ni se solicitó ayuda.
		procesarArchivos "$1"
		echo -e "\nEjecución finalizada:\n"
                echo -e "Se renombraron $contador archivos.\n"
                unset IFS
		exit 0
	fi
fi

if [ $# == 2 ]; then	# Si hay dos parámetros:
 	if [ "$1" != '-r' ]; then # Ejecuta recursivamente?
  		echo -e "\n¡ERROR DE INVOCACIÓN!\n"
		echo -e "Ejecute el script con -h, -? o --help para más información.\n"
 		unset IFS
		exit -1
	else
		esDirectorio "$2"
		procesarArchivosREC "$2"
		echo -e "\nEjecución finalizada:\n"
                echo -e "Se renombraron $contador archivos.\n"
                unset IFS
		exit 0
 	fi
fi

if [ $# -gt 2 ]; then # Si hay más de dos parámetros, muestra el mensaje de error.
	echo -e "\n¡ERROR DE INVOCACIÓN!\n"
        echo -e "Cantidad de parámetros incorrecta.\n"
	echo -e "Ejecute el script con -h, -? o --help para más información.\n"
        unset IFS
	exit -1
fi

###################################################################################
###                                FIN DE SCRIPT                                ###
###################################################################################

