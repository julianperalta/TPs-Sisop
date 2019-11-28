<#

.SYNOPSIS
Este Powershell Script realiza producto escalar o suma de matrices

.DESCRIPTION
El script realiza 2 operaciones: La suma de matrices, o el producto de una matriz por un escalar.
Se ingresa la matriz al script mediante un .txt
La salida se guarda en un archivo llamado salida.nombreArchivoEntrada, en el mismo directorio donde se encuentra el script

.PARAMETER Entrada
    Path del archivo de entrada. No se debe realizar validacion por extension de archivo. Se asume que todos los archivos de entrada tienen matrices validas.
    
.PARAMETER Producto
    De tipo entero, recibe el escalar a ser utilizado en el producto escalar. No se puede usar junto con -Suma.
    
.PARAMETER Suma
    Path del archivo de la matriz a sumar a la indicada en -Entrada. No se puede usar junto con -Producto.
#>

<#
INICIO DEL SCRIPT

Nombre del archivo: Ejercicio6.ps1
Trabajo Práctico: 2 - Powershell
Ejercicio: 6
Autores:
    - Carbone, Emanuel
    - De Stefano, Matias
    - Fiorita, Leandro
    - Gentile, Soledad
    - Peralta, Julian
Entrega: #2
#>
[CmdletBinding()]
param ( #Genero dos Sets de Parametros para que los parametros se llamen juntos y se excluyan mutuamente.
    [Parameter(Mandatory, ParameterSetName = 'Producto')] 
    [Parameter(Mandatory, ParameterSetName = 'Suma')]
    [String]
    $Entrada, #Este parametro pertenece a ambos Sets porque se utiliza en ámbos casos

    [Parameter(Mandatory, ParameterSetName = 'Producto')]
    [int]
    $Producto,

    [Parameter(Mandatory, ParameterSetName = 'Suma')]
    [string]
    $Suma
)

function Select-Producto {
    [CmdletBinding()]
    param (
        [Parameter()]
        [Array]
        $Matriz,

        [int]
        $Escalar
    )
    for ($i = 0; $i -lt $Matriz.Count; $i++) { 
        $Matriz[$i] = $Matriz[$i] * $Escalar 
    }    
    return $Matriz
}

function Select-Suma {
    [CmdletBinding()]
    param (
        [Parameter()]
        [array]
        $Matriz1,

        [array]
        $Matriz2
    )
    
    for ($i = 0; $i -lt $Matriz1.Count; $i++) {
        $Matriz1[$i] = ($Matriz1[$i] + $Matriz2[$i])
    }    
    return $Matriz1
}

function Test-Archivos
{
    [CmdletBinding()]
    param (
        [Parameter()]
        [String]
        $Archivo
    )
    $txt = $Archivo -like '*.txt'
    
    if(!$txt){
        Write-Host "Error: El archivo $Archivo no es un archivo .txt"
        exit -1
    }else{
        if(!(Test-Path $Archivo -PathType Leaf)){
            Write-Host "Error: El archivo $Archivo no existe."
            exit -1
        }
    }
}

Test-Archivos $Entrada
$Mat1 = Get-Content -Path $Entrada #Obtengo el contenido del archivo de texto de entrada
$Fil = $Mat1.Length
$Mat1 = $Mat1.Split('|') #Genero un array, separando el contenido del archivo por cada '|'

$Mat1 = for ($i = 0; $i -lt $Mat1.Count; $i++) {   
    [double]$Mat1[$i] #Convierto cada valor del array a un Double
}

if ($PSBoundParameters.ContainsKey('Suma')) { #True si el parametro Suma se usó.
    Test-Archivos $Suma
    $Mat2 = Get-Content -Path $Suma #Preparo la otra matriz como la de arriba
    $Mat2 = $Mat2.Split('|')
    $Mat2 = for ($i = 0; $i -lt $Mat2.Count; $i++) {   
        [double]$Mat2[$i] #Convierto cada valor del array a un Double
    }
    $Mat1 = Select-Suma $Mat1 $Mat2
}   
else { #Si no hay suma hay producto
    $Mat1 = Select-Producto $Mat1 $Producto
}


#Obtengo el nombre para el archivo de salida
$outputFile = Split-Path $Entrada -leaf
$Salida = "salida." + $outputFile
#Le doy formato y guardo la matriz en el archivo
$Col = $Mat1.Length / $Fil


#Creo un array con la cantidad de elementos por fila correspondientes
for ($i = 0; $i -lt $Mat1.Count; $i += $Col){
    $Matriz += ,@($Mat1[$i..($i+$Col-1)])
}

#Veo si existe el archivo de salida, si es así lo elimino
if(Test-Path $Salida){
    Remove-Item $Salida
}

#creo un nuevo archivo de salida
New-Item $Salida 

#Le doy formato a la matriz y la guardo
for ($i = 0; $i -lt $Matriz.Length; $i++){
    $Matriz[$i] -join "|" | Add-Content $Salida
}

Write-Host "Se ha creado el archivo $Salida"
#Fin del Script