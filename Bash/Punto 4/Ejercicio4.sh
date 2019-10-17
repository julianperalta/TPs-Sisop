#!/bin/bash

###################################################################################
###                             INICIO DE SCRIPT                                ###
#Nombre del script: Ejercicio4.sh
#TP: Bash
#Número de ejercicio: 4
#Integrantes:
# Carbone, Emanuel - DNI: 40081161      
# De Stefano, Matías - DNI: 40130248 
# Fiorita, Leandro - DNI: 40012291
# Gentile, Soledad - DNI: 28053027 
# Peralta, Julián - DNI: 40242831
#Entrega: Segunda entrega
###################################################################################

#Contador de líneas de código y comentarios.
#Cuenta la cantidad de líneas que son de código y de comentario pertenecientes a todos los archivos de una cierta extensión en una ruta.

function documentacion {
    echo 'Modo de empleo: Script "directorio" [extension]'
    echo "  -h      Mostrar ayuda"
    echo "directorio: La ruta donde se analizaran los archivos."
    echo "extension: Tipo de archivo a analizar."
    exit 1
}

if [ "$1" == "-h" -o  "$1" == "-help" -o "$1" == "--help" -o "$1" == "--h" ]
then
	documentacion
fi

if [ "$#" -lt 2 ]
then
	echo "Parámetros insuficientes."
	"Parámetros insuficientes." 2> error.txt
	exit 1
fi


x=0 #Cuenta lineas de comentarios "/* */
y=0 #Cuenta las lineas con //
comentariosAux=0
codigos=0
codigoAux=0
aux=0
comentarios=0
totalLineas=0
cantidadArchivos=0
archTemporal="`date`-outputMultiline.txt" #Creo un archivo temporal para almacenar las líneas multicomentadas encontradas.

for archivo in `find "$1" -type f -name "*.$2"`; do

	cantidadArchivos=`expr "$cantidadArchivos" + 1`
	lineasArchivo=`awk 'END {print NR}' "$archivo"`
	totalLineas=`expr $totalLineas + $lineasArchivo`
	#echo "#DEBUG ------ Procesando archivo: \""$archivo"\""
	
	x=`awk 'BEGIN {x = 0} /\/\*/,/\*\// {x++} ; END {print x}' "$archivo"` #Busco las lìneas multicomentadas y las cuento.
	
	awk '/\/\*/,/\*\//' "$archivo" > "$archTemporal" #Almaceno las lineas de texto en el archivo temporal.
	
	#echo "Cantidad de lineas /* */: $x"
	

	y=`awk 'BEGIN {y = 0} /\/\// {y++} ; END {print y}' "$archivo"` #Cuento la cantidad de lineas comentadas individualmente.
	awk '/\/\//' "$archivo" > outputLine.txt

	aux=`awk 'BEGIN {aux = 0} /\/\// {aux++} ; END {print aux}' "$archTemporal"` #Busco la cantidad de líneas comentadas individualmente dentro de las líneas multicomentadas.
	y=`expr $y - $aux` #Si hay alguna línea comentada individualmente dentro de las multicomentadas, las resto, obteniendo el total de líneas comentadas individualmente.
	#echo "Cantidad de lineas // en el texto: $y"


	#Obtención de líneas de código, borrando todos los comentarios, y contando las líneas con texto.
	sed -n '/\/\*/{p;:a;N;/\*\//!ba;s/.*\n/\n/};p' $archivo | tee ./uncommentedMultiline.c > ./stdout.txt
	sed -r 's/\/\*[\s\S]*?\*\/|([^:]|^)\/\/.*$|\/\*.*\*\/|\/\*.*//' uncommentedMultiline.c | tee ./uncommentedCodeFile.c > ./stdout.txt
	grep -v '\*\/$' ./uncommentedCodeFile.c | tee ./uncommentedFinal.c > ./stdout.txt
		
	codigoAux=`awk 'BEGIN {x=0} /\s*[{}()a-zA-Z0-9]|[{}()a-zA-Z0-9]/ {x++} ; END {print x}' ./uncommentedFinal.c`
	

	
	#################

	comentariosAux=`expr $x + $y`
	codigos=`expr $codigos + $codigoAux`
	comentarios=`expr $comentarios + $comentariosAux`
	
	rm "$archTemporal"
	rm ./stdout.txt
	rm ./uncommentedCodeFile.c
	rm ./uncommentedMultiline.c
	rm ./uncommentedFinal.c
	rm outputLine.txt

done

echo "Se han analizado $cantidadArchivos archivos."
if [ $cantidadArchivos -gt 0 ]
then
porcentajeCodigo=`expr $codigos \* 100`
porcentajeCodigo=`expr $porcentajeCodigo \/ $totalLineas`
porcentajeComment=`expr $comentarios \* 100`
porcentajeComment=`expr $porcentajeComment \/ $totalLineas`
porcentajeLineasEnBlanco=`echo "100 - $porcentajeComment" | bc`
porcentajeLineasEnBlanco=`echo "$porcentajeLineasEnBlanco - $porcentajeCodigo" | bc`
echo "El porcentaje de código es del $porcentajeCodigo%."
echo "El porcentaje de comentarios es del $porcentajeComment%."
echo "El porcentaje de lineas en blanco es del $porcentajeLineasEnBlanco%."
echo "La cantidad total de comentarios es de: $comentarios lineas."
echo "La cantidad total de lineas de código es de: $codigos."
echo "Lineas analizadas: $totalLineas"
exit 0
fi
exit 1

###################################################################################
###                                FIN DE SCRIPT                                ###
###################################################################################
