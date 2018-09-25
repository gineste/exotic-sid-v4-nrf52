@ECHO OFF 
cd components
::for /d %%D in (*) do for /d %%D in (%%D) do echo %%~D
dir /A:D /b /S > ..\fileslist.txt
pause
