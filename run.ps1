# RetailNext PowerShell Run Script
$qtDir = "C:\Qt\6.12.0\mingw_64"
$mingwDir = "C:\Qt\Tools\mingw1310_64"

$env:PATH = "$qtDir\bin;$mingwDir\bin;" + $env:PATH

$exePath = Join-Path $PSScriptRoot "build-mingw\RetailNext.exe"

if (!(Test-Path $exePath)) {
    Write-Host "Binary not found. Building first..." -ForegroundColor Yellow
    & (Join-Path $PSScriptRoot "build.ps1")
}

Write-Host ">>> Launching RetailNext..." -ForegroundColor Green
Start-Process -FilePath $exePath
