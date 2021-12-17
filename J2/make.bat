@echo off

if "--%1%--" == "----" goto make-j2
if "--%1%--" == "--j2--" goto make-j2
goto unknown

:make-j2
set output=j2
set c-files=j2-emu.c
set c-files=%c-files% j2-main.c io-ports.c
echo making %output% ...
echo gcc -o %output% %c-files%
gcc -o %output% %c-files%
if "--%2%--" == "----" goto done
j2 -f:j2 -c:1000
goto done

:unknown
echo Unknown make. I know how to make these:
echo.
echo    j2 - makes j2.exe
echo.
echo    NOTE: if arg2 is given it then runs the program

:done
