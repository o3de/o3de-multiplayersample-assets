@echo off

REM 
REM Copyright (c) Contributors to the Open 3D Engine Project.
REM For complete copyright and license terms please see the LICENSE at the root of this distribution.
REM
REM SPDX-License-Identifier: Apache-2.0 OR MIT
REM
REM

:: Launches Maya 2022+, with this Asset Gem as the Maya project,
:: and bootstrapped with O3DE DccScriptingInterface Gem
:: Note: O3DE DCCsi currently only supports windows and Maya 2022+ with Python3+

:: Set up window
TITLE O3DE DCCsi Launch WingIDE 7x
:: Use obvious color to prevent confusion (Grey with Yellow Text)
COLOR 8E

echo.
echo _____________________________________________________________________
echo.
echo ~    O3DE, Asset Gem, DCCsi Maya Launch Env ...
echo _____________________________________________________________________
echo.
echo ~    default envas for O3DE and Maya
echo.

%~d0
cd %~dp0
PUSHD %~dp0

SETLOCAL ENABLEDELAYEDEXPANSION

:: change the relative path up to dev
set ABS_PATH=%~dp0

:: if the user has set up a custom env call it
:: this should allow the user to locally
:: set env hooks like O3DE_DEV or O3DE_PROJECT
:: Put you project env vars and overrides in this file
IF EXIST "%~dp0Env_Dev.bat" CALL %~dp0Env_Dev.bat

:: if a local customEnv.bat exists, run it
IF EXIST "%~dp0Project_Env.bat" CALL %~dp0Project_Env.bat

:: Maya 2022: 3.7.7 (tags/v3.7.7:d7c567b08f, Mar 10 2020, 10:41:24) [MSC v.1900 64 bit (AMD64)]
IF "%DCCSI_PY_VERSION_MAJOR%"=="" (set DCCSI_PY_VERSION_MAJOR=3)
IF "%DCCSI_PY_VERSION_MINOR%"=="" (set DCCSI_PY_VERSION_MINOR=9)
IF "%DCCSI_PY_VERSION_RELEASE%"=="" (set DCCSI_PY_VERSION_RELEASE=7)
:: Override the default maya version
IF "%MAYA_VERSION%"=="" (set MAYA_VERSION=2023)

CALL %O3DE_DEV%\Gems\AtomLyIntegration\TechnicalArt\DccScriptingInterface\Tools\Dev\Windows\Env_DCC_Maya.bat

:: ide and debugger plug
set "DCCSI_PY_DEFAULT=%MAYA_BIN_PATH%\mayapy.exe"
echo     DCCSI_PY_DEFAULT = %DCCSI_PY_DEFAULT%

:: Some IDEs like Wing, may in some cases need acess directly to the exe to operate correctly
set "DCCSI_PY_IDE=%MAYA_BIN_PATH%\mayapy.exe"
echo     DCCSI_PY_IDE = %DCCSI_PY_IDE%

:: add to the PATH here (this is global)
SET PATH=%MAYA_BIN_PATH%;%DCCSI_PY_IDE%;%DCCSI_PY_DEFAULT%;%PATH%

:: the next line sets up too much, I beleive is causing a maya boot failure
::CALL %~dp0..\Env_O3DE_Python.bat

:: shared location for 64bit python 3.7 DEV location
:: this defines a DCCsi sandbox for lib site-packages by version
:: <O3DE>\Gems\AtomLyIntegration\TechnicalArt\DccScriptingInterface\3rdParty\Python\Lib
set "PATH_DCCSI_PYTHON=%PATH_DCCSIG%\3rdParty\Python"
echo     PATH_DCCSI_PYTHON = %PATH_DCCSI_PYTHON%

:: add access to a Lib location that matches the py version (example: 3.7.x)
:: switch this for other python versions like maya (2.7.x)
IF "%PATH_DCCSI_PYTHON_LIB%"=="" (set "PATH_DCCSI_PYTHON_LIB=%PATH_DCCSI_PYTHON%\Lib\%DCCSI_PY_VERSION_MAJOR%.x\%DCCSI_PY_VERSION_MAJOR%.%DCCSI_PY_VERSION_MINOR%.x\site-packages")
echo     PATH_DCCSI_PYTHON_LIB = %PATH_DCCSI_PYTHON_LIB%

:: add to the PATH
SET PATH=%MAYA_BIN_PATH%;%PATH%

:: add all python related paths to PYTHONPATH for package imports
set PYTHONPATH=%DCCSI_MAYA_SCRIPT_PATH%;%PATH_DCCSIG%;%PATH_DCCSI_PYTHON_LIB%;%PYTHONPATH%

:: some of the env is procedural and build off of other settings
:: so calling it again here, makes sure the overrides in the Env_Dev.bat persist as final.
IF EXIST "%~dp0Env_Dev.bat" CALL %~dp0Env_Dev.bat

echo.
echo _____________________________________________________________________
echo.
echo Launching Maya %MAYA_VERSION% for O3DE DCCsi...
echo _____________________________________________________________________
echo.

echo     MAYA_VERSION = %MAYA_VERSION%
echo     DCCSI_PY_VERSION_MAJOR = %DCCSI_PY_VERSION_MAJOR%
echo     DCCSI_PY_VERSION_MINOR = %DCCSI_PY_VERSION_MINOR%
echo     DCCSI_PY_VERSION_RELEASE = %DCCSI_PY_VERSION_RELEASE%
echo     MAYA_LOCATION = %MAYA_LOCATION%
echo     MAYA_BIN_PATH = %MAYA_BIN_PATH%

echo.
echo     PATH = %PATH%
echo.
echo     PYTHONPATH = %PYTHONPATH%
echo.

:: Change to root dir
CD /D %PATH_O3DE_PROJECT%

echo Ready to start Maya
pause

:: Default to the right version of Maya if we can detect it... and launch
IF EXIST "%MAYA_BIN_PATH%\maya.exe" (
    start "" "%MAYA_BIN_PATH%\maya.exe" %*
) ELSE (
    Where maya.exe 2> NUL
    IF ERRORLEVEL 1 (
        echo Maya.exe could not be found
            pause
    ) ELSE (
        start "" Maya.exe %*
    )
)

::ENDLOCAL

:: Restore previous directory
POPD

:END_OF_FILE

exit /b 0