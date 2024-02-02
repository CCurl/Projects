@echo off

if '--%1--' equ '----'      goto help
if '--%1--' equ '--/?--'    goto help
if '--%1--' equ '--?--'     goto help
if '--%1--' equ '--help--'  goto help
if '--%1--' equ '--/help--' goto help
if '--%1--' equ '--c3--' (
	cd \code\c3
	goto endCD
)
if '--%1--' equ '--code--' (
	cd \code
	goto end
)
if '--%1--' equ '--lg--' (
	lazygit
	goto end
)
if '--%1--' equ '--proj--' (
	cd \code\projects
	goto endCD
)
if '--%1--' equ '--pull--' (
	git pull
	goto end
)

echo ccc: command '%1' not found.
echo;
goto help

:endCD
echo Current folder: %CD%
if '--%2--' equ '--pull--' ( git pull )
if '--%2--' equ '--lg--' ( lazygit )
goto end

:help
echo Usage: ccc [command], where [command] is one of:
echo;
echo   c3:    CD to my c3 folder
echo   code:  CD to my code folder
echo   lg:    Execute 'lazygit'
echo   proj:  CD to my Projects folder
echo   pull:  Execute 'git pull'
echo   NOTE:  'pull' or 'lg' can also be provided as a second
echo            parameter for the commands that CD to git folders
echo            e.g. 'ccc c3 pull' or 'ccc proj lg'

:end
