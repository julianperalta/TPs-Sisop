#!/bin/bash

#####################################################
###					INICIO DE SCRIPT 			  ###
# Nombre del script: ejercicio6.sh
# TP: Bash
# Número de ejercicio: 6
# Integrantes:
#	Carbone, Emanuel - DNI: 40081161
#	De Stefano, Matias - DNI: 40130248
#	Fiorita, Leandro - DNI: 40012291
#	Gentile, Soledad - DNI: 28053027
#	Peralta, Julian - DNI: 40242831
# 
#  Entrega: Tercera entrega
#####################################################

documentacion() {
	echo -e "\nejercicio6"
	echo '------------------'
	echo "MODO DE EMPLEO: $0 [DIRECTORIO]"
	echo '-h	Mostrar ayuda.'
	echo 'DIRECTORIO: La ruta que se quiere analizar. Si no se especifica se utilizará el directorio actual.'
	echo -e '------------------\n'
	exit 1
}

# Utilizo getopts para atrapar los flags que se pasen, en este caso -h para la ayuda
while getopts ":h" opt
do
	case "$opt" in
		h) documentacion;;
		\?) echo "Opción inválida. Para ver la ayuda use -h."
		exit 1;;
	esac
done

esDirectorio() {
	if [ ! -d "$1" ]
	then									# Si el parámetro no es un directorio, muestra
		echo "\nParámetro inválido."		# el mensaje de error y se detiene la ejecución
		echo "\"$1\" no es un directorio."
		echo "Ejecute el script con -h para más información.\n"
		exit 1
	fi
}

IFS=''

contarNroFich() {
	# Los ficheros que encuentre en el find en el directorio se
	# los paso al wc con -l que cuenta la cnatida de líneas que devolvió el find
	nro1=$(find "$1" -mindepth 1 | wc -l)
	#nro2=$(find "$1" ! -name . -prune -print | grep -c /)
	return $nro1
}

recorrer() {
	# Hago un find del directorio pasado por parámetro
	# -mindepth 1 es para no listar "." y ".." como directorios
	# -readable sirve para solo listar directorios para los que tengo permiso de lectura (755)
	# -type -d es para listar únicamente directorios

	# -links 2 es lo que solo lista directoros hoja
	# Todos los directorios hoja tienen únicamente 2 "hard links" (o directorios) que son "."  y ".."

	# Uso xargs para pasarle los directorios encontrados a du para ver su tamaño en disco y luego ordeno de mayor a menor
	# head es para agarrar los 10 primeros directorios ordenados (10 más grandes)

	echo -e "\n"

	find "$1" -mindepth 1 -readable -type d -links 2 2>/dev/null | xargs -d '\n' du -h -a 2>/dev/null | sort -rh | head | while read -r linea
	do
		dir=$(echo "$linea" | sed 's/^\S*.//')	# Obtengo el directorio de la línea que estoy leyendo
		tam=$(echo "$linea" | sed 's/\t.*//')	# Obtengo el tamaño de la línea que estoy leyendo
		contarNroFich "$dir"
		nroFich=$?	# El resultado del último comando es la cantidad de ficheros en el directorio

		echo -e "$dir\t$tam\t$nroFich archivos"
	done

	echo -e "\n"
}


if [ $# -eq 0 ]		# Si la cantidad de parámetros es 0 entonces ejecuto
then				# el script en el directorio actual
	recorrer "."
	unset IFS
	exit 0
elif [ $# -eq 1 ] 	# Si la cantidad de parámetros es 1 entonces ejecuto
then				# el script en el directorio pasado por parámetro
	esDirectorio "$1"
	recorrer "$1"
	unset IFS
	exit 0
else				# Si la cantidad de parámetros es mayor a 1 entonces salgo con error
	echo "\nError de invocación."
	echo 'Cantidad de parámtros incorrecta'
	echo "Ejecute el script con -h para más información.\n"
	unset IFS
	exit 1
fi


#####################################################
###					FIN DE SCRIPT 			  	  ###
#####################################################

