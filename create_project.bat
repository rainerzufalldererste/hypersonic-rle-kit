@ECHO OFF

IF [%1]==[] GOTO MANUAL_CONFIG

IF "%1"=="1" GOTO ONE;
IF "%1"=="2" GOTO TWO;
IF "%1"=="3" GOTO THREE;

ECHO INVALID PARAMETER (%1)

:MANUAL_CONFIG
ECHO 1. Visual Studio 2022 Solution
ECHO 2. Visual Studio 2019 Solution
ECHO 3. gmake2 Makefile

CHOICE /N /C:123 /M "[1-3]:"

IF ERRORLEVEL ==3 GOTO THREE
IF ERRORLEVEL ==2 GOTO TWO
IF ERRORLEVEL ==1 GOTO ONE
GOTO END

:THREE
 ECHO Creating gmake2 Makefile...
 premake\premake5.exe gmake2
 GOTO END

:TWO
 ECHO Creating VS2019 Project...
 premake\premake5.exe vs2019
 GOTO END

:ONE
 ECHO Creating VS2022 Project...
 premake\premake5.exe vs2022
 GOTO END

:END
