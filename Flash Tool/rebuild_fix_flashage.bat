@echo off
REM ===================================================================
REM  PoolConnect Installer - Rebuild Corriger Flashage
REM  Version: 1.0.6 - Fix esptool
REM ===================================================================

setlocal enabledelayedexpansion

echo.
echo =============================================
echo  REBUILD - Correction Flashage ESP32
echo =============================================
echo.

REM Detecter Python
set PYTHON_CMD=
py --version >nul 2>&1
if %errorlevel% equ 0 (
    set PYTHON_CMD=py
    set PIP_CMD=py -m pip
) else (
    python --version >nul 2>&1
    if %errorlevel% equ 0 (
        set PYTHON_CMD=python
        set PIP_CMD=pip
    ) else (
        echo [ERREUR] Python non detecte
        pause
        exit /b 1
    )
)

echo [OK] Python detecte : !PYTHON_CMD!
echo.

REM Nettoyer
echo [1/5] Nettoyage...
if exist build rmdir /s /q build 2>nul
if exist dist rmdir /s /q dist 2>nul
if exist *.spec del /q *.spec 2>nul
echo [OK] Nettoyage termine
echo.

REM Installer dependances
echo [2/5] Installation dependances...
echo     Installation de esptool, pyserial, requests...
!PIP_CMD! install --quiet --upgrade esptool pyserial requests
if %errorlevel% neq 0 (
    echo [ERREUR] Installation dependances echouee
    pause
    exit /b 1
)
echo [OK] Dependances installees
echo.

REM Verifier esptool
echo [3/5] Verification esptool...
!PYTHON_CMD! -m esptool version >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERREUR] esptool ne fonctionne pas
    pause
    exit /b 1
)
echo [OK] esptool fonctionne
echo.

REM Installer PyInstaller
echo [4/5] Installation PyInstaller...
!PIP_CMD! install --quiet pyinstaller
echo [OK] PyInstaller installe
echo.

REM Builder avec TOUS les hidden imports
echo [5/5] Build avec tous les modules...
echo     Cette etape prend 3-5 minutes...
echo.

set ICON_PARAM=
if exist icon.ico set ICON_PARAM=--icon "icon.ico"

!PYTHON_CMD! -m PyInstaller ^
  --onefile ^
  --windowed ^
  --name "PoolConnect_Installer" ^
  !ICON_PARAM! ^
  --hidden-import esptool ^
  --hidden-import esptool.cmds ^
  --hidden-import esptool.util ^
  --hidden-import esptool.loader ^
  --hidden-import esptool.bin_image ^
  --hidden-import esptool.elf ^
  --hidden-import esptool.targets ^
  --hidden-import serial ^
  --hidden-import serial.tools ^
  --hidden-import serial.tools.list_ports ^
  --hidden-import tkinter ^
  --hidden-import tkinter.ttk ^
  --hidden-import requests ^
  --hidden-import struct ^
  --hidden-import hashlib ^
  --hidden-import base64 ^
  --hidden-import zlib ^
  poolconnect_installer.py

if %errorlevel% neq 0 (
    echo.
    echo [ERREUR] Build echoue
    pause
    exit /b 1
)

echo.
echo [OK] Build termine !
echo.

REM Verifier
if exist "dist\PoolConnect_Installer.exe" (
    echo =============================================
    echo  BUILD TERMINE AVEC SUCCES !
    echo =============================================
    echo.
    echo Fichier cree : dist\PoolConnect_Installer.exe
    
    for %%A in ("dist\PoolConnect_Installer.exe") do (
        set /a sizeMB=%%~zA / 1048576
        echo Taille : !sizeMB! MB
    )
    
    echo.
    echo MODULES INCLUS :
    echo  [+] esptool (flashage ESP32)
    echo  [+] serial (detection ports)
    echo  [+] tkinter (interface)
    echo  [+] requests (telechargement)
    echo.
    echo PROCHAINES ETAPES :
    echo  1. Tester : dist\PoolConnect_Installer.exe
    echo  2. Verifier detection port
    echo  3. Tester flashage ESP32
    echo.
) else (
    echo [ERREUR] Executable non trouve
)

pause
