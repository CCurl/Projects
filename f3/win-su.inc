; ------------------------------------------------------------------------------
format PE console 

section '.idata' data readable import

include 'win32ax.inc'
library msvcrt, 'msvcrt.dll'
import msvcrt, putchar, 'putchar'

section '.bdata' data readable writable
