; ------------------------------------------------------------------------------
format PE console 
include 'win32ax.inc'

library kernel32, 'kernel32.dll', msvcrt, 'msvcrt.dll'
import kernel32, GetTickCount, 'GetTickCount', ExitProcess, 'ExitProcess'
import msvcrt, putchar, 'putchar', _getch, '_getch', _kbhit, '_kbhit', gets, 'gets'

; section '.code' code readable executable




; section '.idata' data readable import

; library kernel32, 'kernel32.dll', msvcrt, 'msvcrt.dll', conio, 'conio.dll'
; import msvcrt, putchar, 'putchar', _getch, '_getch', _kbhit, '_kbhit', gets, 'gets'
; import kernel32, GetTickCount, 'GetTickCount'

; section '.bdata' data readable writable
