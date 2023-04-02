; ------------------------------------------------------------------------------
format PE console 

section '.idata' data readable import

include 'win32ax.inc'
library msvcrt, 'msvcrt.dll', conio, 'conio.dll'
import msvcrt, putchar, 'putchar', _getch, '_getch', _kbhit, '_kbhit'

section '.bdata' data readable writable
