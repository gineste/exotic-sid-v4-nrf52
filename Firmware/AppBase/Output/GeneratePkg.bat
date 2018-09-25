@ECHO OFF
CLS

ECHO.
ECHO /*********************************
ECHO  * CREATING PACKAGE FILE FOR DFU *
ECHO  *********************************/
ECHO.

ECHO Change Directory...
CD Output

SETLOCAL
SET Line = ""
SET FirstQuote = 0

FOR /F "tokens=* delims=" %%X in ('type ".\..\Version.h"^| Find /i "#define FW_VERSION "') DO SET Line=%%X > NUL

SET SStrg="^""
SET StrgTemp=%Line%&SET Position=0
:loop
SET /a Position+=1
ECHO %StrgTemp%|FINDSTR /b /c:"%SStrg%" > NUL
IF ERRORLEVEL 1 (
SET StrgTemp=%StrgTemp:~1%
IF DEFINED StrgTemp GOTO loop
SET Position=0
)
SET /a FirstQuote=%Position%

ECHO.%Line%>SVersion
FOR %%a IN (SVersion) DO SET /a SVersion=%%~za -2
SET /a SVersion=%SVersion%-%FirstQuote%-1

CALL SET FirmVersion=%%Line:~%FirstQuote%,%SVersion%%%
DEL SVersion

REM Generate Package Zip
ECHO.
ECHO Generating Zip Package...
CALL nrfutil pkg generate --hw-version 52 --application-version 1 --application ..\Objects\SIDv4Release.hex --sd-req 0x9D --key-file privateSIDv4.key Zip\SID4_v%FirmVersion%.zip

ECHO.
