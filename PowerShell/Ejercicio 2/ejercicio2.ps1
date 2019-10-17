<#
.SYNOPSIS
Nombre del archivo: Ejercicio2.ps1
Trabajo Práctico: 2
Ejercicio: 2
Autores:
    - Carbone, Emanuel
    - De Stefano, Matías
    - Fiorita, Leandro
    - Gentile, Soledad
    - Peralta, Julián
    
Entrega: #1

.DESCRIPTION

Este ejercicio informa cuales de los procesos que estan corriendo en el sistema tiene mas de N instancias.
El resultado se mostrara en un listado con el nombre de cada proceso.
#>

#definimos el unico parametro a ingresar obligatorio: la cantidad de instancias que vamos a tomar como minimo
Param(
[Parameter(Mandatory = $True)]
[ValidateNotNullOrEmpty()]
[int]$Cantidad
)

if($Cantidad -lt 2){
    Write-Host "ERROR: La cantidad debe ser un entero mayor a uno."
    exit
}

$procesos = Get-Process | Group-Object -Property Name -NoElement | Where {$_.Count -ge $Cantidad}

$hashTable = @{}

foreach($proceso in $procesos)
{   
    $hashTable.Add($proceso.Name, $proceso.Count)
}

$hashTable.keys

# --FIN SCRIPT--