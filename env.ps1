# Run this in your PowerShell terminal using:
# . .\env.ps1
# This will add Qt, MinGW GCC, CMake, and Ninja to your current terminal PATH.

$qtDir = "C:\Qt\6.12.0\mingw_64"
$mingwDir = "C:\Qt\Tools\mingw1310_64"
$cmakeDir = "C:\Qt\Tools\CMake_64"
$ninjaDir = "C:\Qt\Tools\Ninja"

$env:PATH = "$qtDir\bin;$mingwDir\bin;$cmakeDir\bin;$ninjaDir;" + $env:PATH

Write-Host "RetailNext toolchain paths added to PATH:" -ForegroundColor Green
Write-Host "  CMake: $(Get-Command cmake | Select-Object -ExpandProperty Source)"
Write-Host "  Ninja: $(Get-Command ninja | Select-Object -ExpandProperty Source)"
Write-Host "  G++:   $(Get-Command g++ | Select-Object -ExpandProperty Source)"
