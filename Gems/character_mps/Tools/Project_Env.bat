@echo off
REM 
REM Copyright (c) Contributors to the Open 3D Engine Project.
REM For complete copyright and license terms please see the LICENSE at the root of this distribution.
REM
REM SPDX-License-Identifier: Apache-2.0 OR MIT
REM
REM

:: Sets up environment for O3DE DCC tools and code access

:: Store current dir
%~d0
cd %~dp0
PUSHD %~dp0

:: change the relative path up to dev
set ABS_PATH=%~dp0

:: project name as a str tag
IF "%O3DE_PROJECT_NAME%"=="" (
    for %%I in ("%~dp0.") do for %%J in ("%%~dpI.") do set O3DE_PROJECT_NAME=%%~nxJ
    )

echo.
echo _____________________________________________________________________
echo.
echo ~    Setting up O3DE DSI %O3DE_PROJECT_NAME% Environment ...
echo _____________________________________________________________________
echo.

:: if the user has set up a custom env call it
IF EXIST "%~dp0Env_Dev.bat" CALL %~dp0Env_Dev.bat

:: O3DE_PROJECT is ideally treated as a full path in the env launchers
:: do to changes in o3de, external engine/project/gem folder structures, etc.
IF "%O3DE_PROJECT%"=="" (
    for %%i in ("%~dp0..") do set "O3DE_PROJECT=%%~fi"
    )
echo     O3DE_PROJECT = %O3DE_PROJECT%

:: this is here for archaic reasons, WILL DEPRECATE
IF "%PATH_O3DE_PROJECT%"=="" (set PATH_O3DE_PROJECT=%O3DE_PROJECT%)
echo     PATH_O3DE_PROJECT = %PATH_O3DE_PROJECT%

set "PATH_O3DE_CACHE=%PATH_O3DE_PROJECT%\Cache"
echo     PATH_O3DE_CACHE = %PATH_O3DE_CACHE%

IF "%O3DE_DEV%"=="" (
    echo       ~ O3DE_DEV is not set aka engine_root
    echo       ~ set O3DE_DEV in Dev.bat
    echo       ~ example: set "O3DE_DEV=C:\O3DE\0.0.0.0"
    set O3DE_DEV=C:\not\set\o3de
    )

CALL %O3DE_DEV%\Gems\AtomLyIntegration\TechnicalArt\DccScriptingInterface\Tools\Dev\Windows\Env_O3DE_Core.bat
CALL %O3DE_DEV%\Gems\AtomLyIntegration\TechnicalArt\DccScriptingInterface\Tools\Dev\Windows\Env_O3DE_Python.bat

:: add to the PATH here (this is global)
SET PATH=%PATH_O3DE_BIN%;%PATH_DCCSIG%;%PATH%

:: Restore original directory
popd

:: Change to root dir
CD /D %ABS_PATH%

GOTO END_OF_FILE

:: Return to starting directory
POPD

:END_OF_FILE