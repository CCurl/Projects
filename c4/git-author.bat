@echo off
echo xx | set /p=%1
echo xx | set /p=:
git log --pretty="%%aN,%%as" --reverse %1 | c4 top.c4 1