@echo off

if '--%1--' equ '--/?--' goto help
if '--%1--' equ '--?--' goto help
if '--%1--' equ '--help--' goto help
if '--%1--' equ '--/help--' goto help

if '--%1--' neq '--c3--' goto n1
	cd \code\c3
goto endCD

:n1
if '--%1--' neq '--proj--' goto n2
	cd \code\projects
goto endCD

:n2
if '--%1--' neq '--code--' goto n3
	cd \code
goto endCD

:n3
if '--%1--' neq '--pull--' goto nf
	git pull
goto end

:nf
echo ccc: command '%1' not found.

:help
echo Usage: ccc command
echo;
echo   c3:    cd to my c3 folder
echo   code:  cd to my code folder
echo   proj:  cd to my Projects folder
echo   pull:  execute a 'git pull'
echo   NOTE:  'pull' can also be provided as a second
echo          parameter for the commands that change folders
echo          e.g. ccc c3 pull
goto end

:endCD
cd
if '--%2--' neq '--pull--' goto end
	git pull

:end
