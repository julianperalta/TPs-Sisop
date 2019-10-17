<#
.Synopsis
Nombre del archivo: Ejercicio5.ps1
Trabajo Práctico: 2
Ejercicio: 5
Autores:
    - Carbone, Emanuel
    - De Stefano, Matías
    - Fiorita, Leandro
    - Gentile, Soledad
    - Peralta, Julián
Entrega: #1

.DESCRIPTION
   Este script puede contar la cantidad de procesos corriendo en el sistema, o
   mostrar el peso de un directorio cada 10 segundos.

.PARAMETER Procesos
    Indica al script que debe mostrar la cantidad de procesos actuales en el sistema.

.PARAMETER Peso
    Indica al script que debe verificar el peso del directorio actual (en MB).

.PARAMETER Directorio
    Establece ruta a analizar su peso (en MB).

#>


Param(
        [parameter(mandatory=$true, ParameterSetName = 'Procesos')][switch]$Procesos,
        [parameter(mandatory=$true, ParameterSetName = 'PesoDirectorio')][switch]$Peso,
        [parameter(mandatory=$false, ParameterSetName = 'PesoDirectorio')][string]$Directorio
     )

function Get-Procesos()
{
    $cantidadProcesos = Get-Process | Measure
    return $cantidadProcesos.Count
}

#Sección de validación (Utilizando ParameterSetName no debería entrar por acá)

if($Procesos -and $Peso)
{
    Write-Output "Solo se puede usar '-procesos' o '-peso'. No ambos."
    exit
}

if($Procesos -and $Directorio)
{
    Write-Output "Solo se puede enviar un directorio si se utiliza el parámetro -peso"
    exit
}

#Sección de ejecución

while ($true) {

    if($Procesos)
    {
        $retorno = Get-Procesos
        Write-Output $retorno
    }

    if($Peso)
    {
        if($Directorio)
        {
            $tamCarpeta = "{0:N2}" -f ((Get-ChildItem -Recurse $Directorio | Measure-Object -Property Length -s).Sum / 1Mb)
            Write-Output $tamCarpeta
        }
        else
        {
            $tamCarpeta = "{0:N2}" -f ((Get-ChildItem -Recurse $PWD | Measure-Object -Property Length -s).Sum / 1Mb)
            Write-Output $tamCarpeta
        }
    }

    Start-Sleep -Seconds 10 
} 

<# FIN DE SCRIPT #>