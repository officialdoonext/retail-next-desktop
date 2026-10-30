# RetailNext PowerShell Build Script
# Automatically configures environment for installed Qt 6 MinGW kit and builds the project

Stop-Process -Name RetailNext -Force -ErrorAction SilentlyContinue

$qtDir = "C:\Qt\6.12.0\mingw_64"
$mingwDir = "C:\Qt\Tools\mingw1310_64"
$cmakeDir = "C:\Qt\Tools\CMake_64"
$ninjaDir = "C:\Qt\Tools\Ninja"

$env:PATH = "$qtDir\bin;$mingwDir\bin;$cmakeDir\bin;$ninjaDir;" + $env:PATH

Write-Host ">>> Configuring RetailNext with CMake & MinGW Makefiles..." -ForegroundColor Cyan
cmake -B build-mingw -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$qtDir"

if ($LASTEXITCODE -ne 0) {
    Write-Error "CMake configuration failed."
    exit $LASTEXITCODE
}

Write-Host ">>> Compiling RetailNext..." -ForegroundColor Cyan
cmake --build build-mingw --parallel 4

if ($LASTEXITCODE -ne 0) {
    Write-Error "Compilation failed."
    exit $LASTEXITCODE
}

Write-Host ">>> Deploying Qt runtime libraries (windeployqt)..." -ForegroundColor Cyan
& "$qtDir\bin\windeployqt.exe" --release build-mingw\RetailNext.exe

Write-Host ">>> Build completed successfully! Binary is at build-mingw\RetailNext.exe" -ForegroundColor Green
