@ECHO OFF
CLS

IF EXIST "C:\Program Files\Git\bin\" SET PATH=%PATH%;C:\Program Files\Git\bin\

IF EXIST "C:\MinGW\msys\1.0\bin\" SET PATH=%PATH%;C:\MinGW\msys\1.0\bin\

IF EXIST "C:\ti\ccsv6\utils\cygwin" SET PATH=%PATH%;C:\ti\ccsv6\utils\cygwin\

REM ECHO %PATH%

REM SET mypath=%~dp0
REM echo %mypath:~0,-1%

CALL sh ChangeVersion.sh
