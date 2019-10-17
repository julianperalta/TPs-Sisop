<#
.SYNOPSIS
Nombre del archivo: Ejercicio3.ps1
Trabajo Práctico: 2
Ejercicio: 3
Autores:
    - Carbone, Emanuel
    - De Stefano, Matías
    - Fiorita, Leandro
    - Gentile, Soledad
    - Peralta, Julián
Entrega: #1

.DESCRIPTION
   Este script mueve los archivos de un directorio a otro mediante un archivo CSV que indica las rutas de destino. Finalmente escribe un archivo CSV de salida (un log) con todos los movimientos realizados y la fecha de ejecución.

.PARAMETER Entrada
    Path del archivo CSV de entrada.

.PARAMETER Salida
    Path del archivo CSV de salida.

.EXAMPLE
    ./Ejercicio3.ps1 -Entrada "./entrada.csv" -Salida "./salida/Log.csv"
    
    ./Ejercicio3.ps1 -Entrada "./entrada.csv" -Salida "/home/path/absoluto/salida/Log.csv"

    ./Ejercicio3.ps1 -Entrada "/home/path/absoluto/archivo/entrada.csv" -Salida "./salida/Log.csv"

    ./Ejercicio3.ps1 -Entrada "/home/path/absoluto/archivo/entrada.csv" -Salida "/home/path/absoluto/salida/Log.csv"
#>

Param(
    [Parameter(Mandatory, ParameterSetName = 'Csv')][string]$Entrada,
    [Parameter(Mandatory, ParameterSetName = 'Csv')][string]$Salida
)

function Ayuda() {
    Write-Host "`nSe debe hacer uso de los dos parámetros, '-Entrada' y '-Salida'"
    Write-Host "Para más información utilice:`n"
    Write-Host "1)" -NoNewline
    Write-Host -ForegroundColor Yellow "`tGet-Help " -NoNewline
    Write-Host "./Ejercicio3.ps1`t`n"
    Write-Host "2)`t./Ejercicio3.ps1 " -NoNewline
    Write-Host -ForegroundColor Gray "-?`n"
}

# Validación de parámetros
if(!$Entrada -or !$Salida) {
    Ayuda
    exit -1
}

if($Entrada -and $Salida) {
    If(-not (Test-Path -Path $Entrada)) {
        Write-Host "`nEl parámetro pasado en '-Entrada' no es un path válido.`n"
        exit -1
    }
    ElseIf(-not (Test-Path -Path $Salida)) {
        Write-Host "`nEl parámetro pasado en '-Salida' no es un path válido.`n"
        exit -1
    }
}

function ExportarCsv() {
    Param(
        [Parameter(Mandatory, ParameterSetName = 'Csv')]
        [psobject[]]
        $FilasCSV
    )

    try {
        $FilasCSV | Export-Csv -Path "$Salida" -Delimiter ',' -Append -NoTypeInformation
    }
    catch {
        $msjError = $_.Exception.Message
        Write-Host -ForegroundColor Red -BackgroundColor Black $msjError
    }
}

function MoverArchivos() {
    
    $csvEntrada = Import-Csv -Path "$Entrada" -Delimiter ','

    $filasCsvSalida = @()

    foreach ($item in $csvEntrada) {
        $origen = $item.origen
        $destino = $item.destino

        $propiedades = [ordered] @{
            archivo = $destino
            fecha = Get-Date -Format 'dd/MM/yyyy'
        }
        $nuevaFila = New-Object -TypeName psobject -Property $propiedades
        $filasCsvSalida += $nuevaFila

        Move-Item -Path "$origen" -Destination "$destino"
    }

    ExportarCsv -FilasCSV $filasCsvSalida
}

MoverArchivos

################################
###      FIN DE SCRIPT       ###
################################