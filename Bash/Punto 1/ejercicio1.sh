#!/bin/bash

###################################################################################
###				INICIO DE SCRIPT				###
#Nombre del script: ejercicio1.sh
#TP: Bash
#Número de ejercicio: 1
#Integrantes:
# Carbone, Emanuel - DNI: 40081161 	
# De Stefano, Matías - DNI: 40130248 
# Fiorita, Leandro - DNI: 40012291
# Gentile, Soledad - DNI: 28053027 
# Peralta, Julián - DNI: 40242831
#Entrega: Primera entrega
###################################################################################

# Señala que se produjo un error de sintaxis y cómo se ejecuta el script.
ErrorS()
{
 echo "Error. La sintaxis del script es la siguiente:"
 echo "Contar líneas del archivo: $0 nombre_espacio L"
 echo "Contar la cantidad de bytes del archivo: $0 nombre_espacio C"
 echo "Longitud de la línea más extensa: $0 nombre_espacio M" 
}

# Señala un error específico a la hora de ejecutar.
ErrorP()
{
 echo "Error. nombre_archivo no existe o no se posee permiso de lectura" 
}

# Si se pasaron menos de dos parámetros, muestra el correspondiente mensaje de error.
if test $# -lt 2; then
 ErrorS
fi

# Comprueba que el archivo exista y se tengan permisos de lectura,
# caso contrario, señala el error correspondiente.
if ! test -r $1; then
 ErrorP

# Comprueba que el archivo sea un archivo regular y que el segundo parámetro sea válido.
elif test -f $1 && (test $2 = "L" || test $2 = "C" || test $2 = "M"); then

# Si el parámetro fue L, muestra la cantidad de líneas del archivo.
 if test $2 = "L"; then
  res=`wc -l $1`
  echo "Cantidad de líneas del archivo: $res"

# Si el parámetro fue C, muestra la cantidad de bytes del archivo.
 elif test $2 = "C"; then
  res=`wc -m $1`
  echo "Cantidad de bytes del archivo: $res"

# Si el parámetro fue M, muestra la longitud de la línea más extensa.
 elif test $2 = "M"; then
  res=`wc -L $1`
  echo "Longitud de la línea más extensa: $res"
 fi

# El archivo o los parámetros no son válidos,
# entonces se señala el error correspondiente.
else
 ErrorS
fi

# a) Dado un archivo pasado por parámetro, el script puede:
# - Contar la cantidad de lineas que tiene. (-L)
# - Contar la cantidad de bytes que tiene. (-C)
# - Mostrar el largo de la línea más extensa. (-M)

# b) Los parámetros que recibe son: 
# - El nombre de un archivo.
# - Una operación a realizar (-L, -C o -M)

# e) La variable $# contiene la cantidad de parámetros que recibió.
# Otras variables especiales pueden ser:
# $0: Nombre del script.
# $$: PID del shell.
# $*: linea de llamada del PROMPT.
# $!: PID del ultimo comando ejecutado.
# $@: todos los parametros.
# $?: valor de retorno del ultimo comando.

# f) Los distintos tipos de comillas son:
# - Comillas débiles("): El intérprete puede analizar el contenido del string,
# y en base a eso, operar.
# - Comillas fuertes('): El contenido se toma como un string literal.
# - Comillas de ejecución(`): El intérprete intenta ejecutar lo que hay dentro.

###################################################################################
###                                FIN DE SCRIPT                                ###
###################################################################################
