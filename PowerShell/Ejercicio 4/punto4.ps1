<#
.SYNOPSIS
    El script permite comprimir y descomprimir directorios así como también
    informar sobre la relación de compresión de un .zip 

.DESCRIPTION
    El Script posee tres modos: Comprimir, Descomprimir e Informar. 
    En el primer modo, comprimirá un directorio bajo un nombre indicado.
    En el segundo modo, descomprimirá un .zip en un directorio indicado.
    En el tercer modo, mostrará información de compresión de un .zip indicado.

.PARAMETER PathZip
    Ruta del archivo .zip a crear, creado o para analizar, según corresponda.
.PARAMETER Directorio
    Ruta al directorio origen o destino, según corresponda.
.PARAMETER Comprimir
    Modo de compresión.
.PARAMETER Descomprimir
    Modo de descompresión.
.PARAMETER Informar
    Modo de información.

.EXAMPLE
    punto4.ps1 -PathZip directorio.zip -Directorio directorioAComprimir -Comprimir
.EXAMPLE
    punto4.ps1 -PathZip directorioADescomprimir.zip -Directorio directorioDestino -Descomprimir
.EXAMPLE
    punto4.ps1 -PathZip directorioAAnalizar.zip -Informar
#>

<# 
    --INICIO DEL SCRIPT--
Nombre del archivo: punto4.ps1
Trabajo Práctico: PowerShell
Ejercicio: 4
Autores:
    - Carbone, Emanuel (DNI: 40081161)
    - De Stefano, Matías (DNI: 40130248)
    - Fiorita, Leandro (DNI: 40012291)
    - Gentile, Soledad (DNI: 28053027)
    - Peralta, Julián (DNI: 40242831)
Entrega: #1
#>

# Configuración de parámetros de entrada
Param
(
 [parameter(Mandatory=$true)][ValidateNotNullOrEmpty()][string]$PathZip,
 [Parameter(ParameterSetName='Comprimir', Mandatory=$true)]
 [Parameter(ParameterSetName='Descomprimir', Mandatory=$true)]
 [ValidateNotNullOrEmpty()][string]$Directorio,
 [Parameter(ParameterSetName='Comprimir', Mandatory=$true)][switch]$Comprimir,
 [Parameter(ParameterSetName='Descomprimir', Mandatory=$true)][switch]$Descomprimir,
 [Parameter(ParameterSetName='Informar', Mandatory=$true)] [switch]$Informar
)

# Agregado de la clase ZipFile
Add-Type -AssemblyName 'System.IO.Compression.FileSystem'

# Comprueba si el directorio existe
function Test-Directorio {
    
    $existeDirectorio = Test-Path $Directorio -PathType Container

    if($existeDirectorio -eq $false){
        Write-Host "ERROR: El directorio $Directorio no existe."
        exit
    }       
}

# Comprueba si el archivo es un .zip y si existe
function Test-PathZip {

    $esZip = $PathZip -like '*.zip'
    
    if($esZip -eq $false){
        Write-Host "ERROR: El archivo $PathZip no es un .zip."
        exit
    }
    else{
        $existeZip = Test-Path $PathZip -PathType Leaf
        if($existeZip -eq $false){
            Write-Host "ERROR: El archivo $PathZip no existe."
            exit
        }
    }
}
       
 if($Comprimir) { # Modo Comprimir 
    Test-Directorio  # Existe $Directorio?
    Compress-Archive -Path "$Directorio" -Force -CompressionLevel Optimal -DestinationPath "$PathZip"; # Comprimo
    Write-Host "¡Se comprimió exitosamente!";
 }
 elseif($Descomprimir) { # Modo Descomprimir 
    Test-PathZip # Existe $Directorio?
    Test-Directorio # Es $PathZip un .zip y existe? 
    Expand-Archive -Path "$PathZip" -Force -DestinationPath "$Directorio"; # Descomprimo
    Write-Host "¡Se descomprimió exitosamente!";
 }
 elseif($Informar) { # Modo Informar
    Test-PathZip # Es $PathZip un .zip y existe?

    # Construyo ruta absoluta de $PathZip
    $PathZip = (Get-ChildItem -Path $PathZip).FullName
    Write-Host ""
    Write-Host "--Información de $PathZip--"

    # Obtengo archivos del .zip
    $archivosZip = [System.IO.Compression.ZipFile]::Open("$PathZip", 'Read')

    # Por cada archivo del .zip, calculo e informo
    foreach($archivo in $archivosZip.Entries){
        
        [pscustomobject]@{
                Archivo = $archivo.FullName
                Peso_Comprimido = $archivo.CompressedLength
                Peso = $archivo.Length
                Relacion = "{0:P2}" -f ([long]$archivo.CompressedLength / [long]$archivo.Length)
        } 
    }
 }

#   --FIN DEL SCRIPT--