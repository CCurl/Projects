@echo off

if '--%1--' equ '----'      goto help
if '--%1--' equ '--/?--'    goto help
if '--%1--' equ '--?--'     goto help
if '--%1--' equ '--help--'  goto help
if '--%1--' equ '--/help--' goto help
if '--%1--' equ '--c3--'    goto c3
if '--%1--' equ '--code--'  goto code
if '--%1--' equ '--lg--'    goto lg
if '--%1--' equ '--proj--'  goto proj
if '--%1--' equ '--pull--'  goto pull

echo ccc: command '%1' not found.
echo;
goto help

:c3
cd \code\c3
goto endCD

:proj
cd \code\projects
goto endCD

:code
cd \code
goto endCD

:pull
git pull
goto end

:lg
lazygit
goto end

:endCD
cd
if '--%2--' equ '--pull--' goto pull
if '--%2--' equ '--lg--' goto lg
goto end

:help
echo Usage: ccc [command], where [command] is one of:
echo;
echo   c3:    cd to my c3 folder
echo   code:  cd to my code folder
echo   lg:    execute 'lazygit'
echo   proj:  cd to my Projects folder
echo   pull:  execute 'git pull'
echo   NOTE:  'pull' or 'lg' can also be provided as a second
echo            parameter for the commands that change folders
echo            e.g. 'ccc c3 pull' or 'ccc proj lg'

:end
