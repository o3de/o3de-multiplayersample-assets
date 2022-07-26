@echo off
REM
REM Copyright (c) Contributors to the Open 3D Engine Project.
REM For complete copyright and license terms please see the LICENSE at the root of this distribution.
REM
REM SPDX-License-Identifier: Apache-2.0 OR MIT
REM
REM

echo.
echo _____________________________________________________________________
echo.
echo ~    O3DE DCCsi Env Dev ...
echo _____________________________________________________________________
echo.

:: set O3DE_BUILD_FOO=True
::echo     O3DE_BUILD_FOO = %O3DE_BUILD_FOO%

set O3DE_DEV=c:\depot\o3de-dev
echo     O3DE_DEV = %O3DE_DEV%

set O3DE_BUILD_FOLDER=build
echo     O3DE_BUILD_FOLDER = %O3DE_BUILD_FOLDER%

set "PATH_O3DE_BUILD=%O3DE_DEV%\%O3DE_BUILD_FOLDER%"
echo     PATH_O3DE_BUILD = %PATH_O3DE_BUILD%

set "PATH_O3DE_BIN=%PATH_O3DE_BUILD%\bin\profile"
echo     PATH_O3DE_BIN = %PATH_O3DE_BIN%

set "PATH_O3DE_CACHE = C:\depot\o3de-multiplayersample\Cache"
echo     PATH_O3DE_CACHE = %PATH_O3DE_CACHE%

set DCCSI_GDEBUG=False
echo     DCCSI_GDEBUG = %DCCSI_GDEBUG%

set DCCSI_DEV_MODE=False
echo     DCCSI_DEV_MODE = %DCCSI_DEV_MODE%

set DCCSI_GDEBUGGER=WING
echo     DCCSI_GDEBUGGER = %DCCSI_GDEBUGGER%

set DCCSI_LOGLEVEL=10
echo     DCCSI_LOGLEVEL = %DCCSI_LOGLEVEL%

:: Update these if you want to run a newer version of Maya
:: Note: O3DE DCCsi currently only supports windows and Maya 2022+ with Python3+
:: Maya 2022: 3.7.7 (tags/v3.7.7:d7c567b08f, Mar 10 2020, 10:41:24) [MSC v.1900 64 bit (AMD64)]
set DCCSI_PY_VERSION_MAJOR=3
set DCCSI_PY_VERSION_MINOR=7
set DCCSI_PY_VERSION_RELEASE=7
:: Override the default maya version
set MAYA_VERSION=2022
