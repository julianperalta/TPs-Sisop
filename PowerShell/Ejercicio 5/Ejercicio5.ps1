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
Entrega: #2

.DESCRIPTION
   Este script puede contar la cantidad de procesos corriendo en el sistema, o
   mostrar el peso de un directorio cada 1 segundo.

.EXAMPLE
        .\Ejercicio5.ps1 -Procesos
        201
.EXAMPLE
        .\Ejercicio5.ps1 -Peso
        3050
.EXAMPLE
        .\Ejercicio5.ps1 -Peso "./Pruebas/test1"
        45030
.EXAMPLE
        .\Ejercicio5.ps1 -Peso -Directorio ".."
        23467892

.PARAMETER Procesos
    Indica al script que debe mostrar la cantidad de procesos actuales en el sistema.

.PARAMETER Peso
    Indica al script que debe verificar el peso del directorio actual.

.PARAMETER Directorio
    Establece ruta a analizar su peso.

#>


Param(
        [parameter(mandatory=$true, Position = 0, ParameterSetName = 'Procesos')][switch]$Procesos,
        [parameter(mandatory=$true, Position = 0, ParameterSetName = 'PesoDirectorio')][switch]$Peso,
        [parameter(mandatory=$false,Position = 1, ParameterSetName = 'PesoDirectorio')][string]$Directorio
     )

function Get-Procesos()
{
    $cantidadProcesos = Get-Process | Measure
    return $cantidadProcesos.Count
}

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

if($Procesos)
{
    while($true)
    {
        $retorno = Get-Procesos
        Write-Output $retorno
        Start-Sleep -Seconds 1 
    }
        
}

if($Peso)
{
    if($Directorio)
    {
        $directorioValido = Test-Path -Path $Directorio
        if($directorioValido -eq $true)
        {
            while($true)
            {
                $tamCarpeta = (Get-ChildItem -Recurse $Directorio | Measure-Object -Property Length -Sum).Sum
                Write-Output $tamCarpeta
                Start-Sleep -Seconds 1
            }   
        }
        else
        {
            Write-Output "El directorio especificado no es valido."
        }
    }
    else
    {
        while($true)
        {
            $tamCarpeta = (Get-ChildItem -Recurse $PWD | Measure-Object -Property Length -Sum).Sum
            Write-Output $tamCarpeta
            Start-Sleep -Seconds 1
        }
        
    }
}
<# FIN DE SCRIPT #>