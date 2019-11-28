#!/bin/bash

#Ejercicio 5 Bash - Script que simula el comando rm implementando una papelera de reciclaje

#Integrantes:
# Carbone, Emanuel - DNI: 40081161 	
# De Stefano, Matías - DNI: 40130248 
# Fiorita, Leandro - DNI: 40012291
# Gentile, Soledad - DNI: 28053027 
# Peralta, Julián - DNI: 40242831
#Entrega: Tercer reentrega

Ayuda () {
echo -e "
		El script \"Ejercicio5\" emula el comportamiento del comando rm, pero utilizando un concepto de \"papelera de reciclaje\".
		Los archivos borrados se podrán recuperar excepto que la papelera se hubiera vaciado anteriormente.
		
		El script tendrá las siguientes opciones:
			-l listar los archivos que contiene la papelera de reciclaje.
			-r <nombreArchivo> recuperar el archivo pasado por parámetro a su ubicación original.
			-e vaciar la papelera de reciclaje (eliminar definitivamente)
		Sin modificador para que elimine el archivo.

		Modo de uso:
			./ejercicio5.sh  <pathArchivo> | [-l] | [-r <nombreArchivo>] | [-e]
		
		La papelera está alojada en "$homeUser", en un directorio oculto.\n" 
}

homeUser=$(eval echo ~$USER)
dirPapelera="${homeUser}/.Papelera"

Verificar_Papelera (){

	if [ -d "$dirPapelera" ];
	then
		: echo " "
	else
		mkdir -m777 "$dirPapelera"
	fi
}

Restauracion (){
	fileAux=$i #Creo una variable auxiliar para almacenar el path
	
	fileAux="${fileAux#${pathParcial}}" #En la variable auxiliar, borro la parte perteneciente a la ruta de la papelera, para obtener el path original.
	#fileAux=".""$fileAux" #Agrego el punto para que el archivo se mueva a home
	dirAux="$(dirname "${fileAux}")" #Obtengo el path del directorio, sin el nombre del archivo
	dirAux="$homeUser""$dirAux"
	echo "¿Desea restaurar el archivo perteneciente a la siguiente dirección? <1 = SI; 0 = NO>
	$fileAux"
	read input
	if [ $input -eq 1 ] #ingreso por teclado la opcion, si no es 1 no lo restaura
		then 
			mkdir -p "$dirAux" ; mv -i "$i" "$dirAux" #Si el directorio no existe, lo crea, y luego mueve el archivo
			echo "Archivo restaurado correctamente"
	fi
}

Restaurar_Archivo (){
	param="$1"
	if [ ! "$param" ]
		then
			echo "No se ha especificado un archivo a restaurar"
		else
			#Creo un array de los resultados 
			
			IFS=$'\n'
			resultados=($(find "$dirPapelera" -type f -name "$param"))
			
			if [ ${#resultados[@]} -gt 1 ]; #Compruebo que el array tenga más de 1 resultado
				then
					echo "Se ha encontrado más de un archivo eliminado con el nombre "$param""

					for i in ${resultados[@]} #Recorro el array y muestro los resultados
						do
							Restauracion
							rmdir "$(dirname ${i})"  #Elimino los directorios que quedaron vacios
						done	
				else if [ ${#resultados[@]} -eq 1 ]
					then
					echo ${resultados[0]}
						i=${resultados[0]} #Recorro el array y muestro los resultados
						Restauracion
						rmdir "$(dirname "${i}")"  #Elimino los directorios que quedaron vacios
					else
						echo "No se ha encontrado ningún archivo con el nombre \"$param\""
					fi
			fi
			unset IFS
	fi
	
}

Eliminar_Archivo (){
	archivo="$1"
	if [ -f "$archivo" ] #Verifico si el archivo existe
	then
		if [[ "${archivo:0:1}" = "/" ]]
		then
			: echo "ruta abs"
		else
			echo $pwd
			archivo=$PWD"/"${archivo}
		fi
		pathHome="$homeUser" #Guardo la direccion del /home/user/
		pathAux="${archivo#$pathHome}" #Le saco al path del archivo el pathHome
		#pathAux="$dirPapelera""$(dirname "${pathAux}")" #Concateno a la Papelera el path obtenido en la linea anterior	
		pathAux="$dirPapelera$(dirname "${pathAux}")" 

		mkdir -p "$pathAux"; mv "$archivo" "$pathAux" #Creo el path en la papelera si no existe, luego muevo el archivo
		echo "Se ha eliminado el archivo"
	else
		echo "El archivo no existe o la opción ingresada no es correcta"
	fi
}

Verificar_Papelera

pathParcial="$dirPapelera"

while [ -n "$1" ]; do # Verificación de opciones

	case "$1" in
	
	-l) 
		if [ $# -gt 1 ]
		then
			echo -e "Hay más parametros de los indicados para esta opción."
			echo -e "Ejecute el script con -h, -? o --help para más información."
			exit 1
		fi
		echo "Se listarán los archivos de la papelera"
		ls -R "$dirPapelera"
		#ls -R "$dirPapelera" | grep -v /
		exit;;
	
	-r) if [ $# -gt 2 ]
		then
			echo -e "Hay más parametros de los indicados para esta opción."
			echo -e "Ejecute el script con -h, -? o --help para más información."
		exit 1
		fi
		Restaurar_Archivo "$2"
		exit;;
	
	-e) 
		if [ $# -gt 1 ]
		then
			echo -e "Hay más parametros de los indicados para esta opción."
			echo -e "Ejecute el script con -h, -? o --help para más información."
		exit 1
		fi
		echo "Se vaciará la papelera" 
		rm -r "$dirPapelera/"
		exit;;
	
	-h | --help | -?) Ayuda
		exit;;
		
	*) 	if [ $# -gt 1 ]
		then
			echo -e "Hay más parametros de los indicados para esta opción."
			echo -e "Ejecute el script con -h, -? o --help para más información."
		exit 1
		fi
		Eliminar_Archivo "$1"
		exit;;
	esac
	
	shift
	
done 

##Fin del Script
