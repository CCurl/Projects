; I/O support for Linux
; ------------------------------------------------------------------------------

macro PUTc {
    pop eax
    mov	[buf4], al  ; put char in message
    mov	ecx,buf4    ; message to write
    mov	edx,1		; message length
    mov	ebx,1		; file descriptor (stdout)
    mov	eax,4		; system call number (sys_write)
    int	0x80		; call kernel
 }

macro TRC ch
{
    push ch
    PUTc
}

s_EMIT:
    PUTc
    NEXT
