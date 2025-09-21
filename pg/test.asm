; 4 instructions removed
format ELF executable
;================== code =====================
segment readable executable
start:
	LEA  EBP, [locals]
	CALL F10
;================== library ==================
exit:
	MOV EAX, 1
	XOR EBX, EBX
	INT 0x80

;=============================================

F9: ; func
	MOV  EAX, [EBP+4]
	ADD  EAX, 2
	MOV  [EBP+8], EAX
	MOV  EAX, [EBP+8]
	ADD  EAX, 3
	MOV  [EBP+12], EAX
	RET

F10: ; main
	ADD  EBP, 40
	MOV  EAX, 4
	MOV  [EBP+4], EAX
	CALL F9 ; func
	MOV  EAX, 0
	PUSH EAX
	MOV  EAX, [EBP+12]
	POP  ECX
	MOV  DWORD [I6 + ECX*4], EAX ; a
	SUB  EBP, 40
	MOV  EAX, [I6] ; a
	INC  [I6] ; a
	PUSH EAX
	MOV  EAX, 97
	POP  ECX
	MOV  BYTE [C5 + ECX*1], AL ; y
	MOV  EAX, 2
	PUSH EAX
	MOV  EAX, [I6] ; a
	ADD  EAX, 223
	POP  ECX
	MOV  BYTE [C5 + ECX*1], AL ; y
	MOV  EAX, [I6] ; a
	PUSH EAX
	MOV  EAX, 123
	POP  ECX
	MOV  BYTE [C5 + ECX*1], AL ; y
	MOV  EAX, [I6] ; a
	ADD  EAX, 1
	PUSH EAX
	MOV  EAX, 0
	POP  ECX
	MOV  BYTE [C5 + ECX*1], AL ; y
	MOV  EAX, 0
	PUSH EAX
	MOV  EAX, 1
	POP  ECX
	MOV  DWORD [I4 + ECX*4], EAX ; xx
	MOV  EAX, [I4] ; xx
	INC  [I4] ; xx
	PUSH EAX
	MOV  EAX, [I6] ; a
	POP  ECX
	MOV  DWORD [I4 + ECX*4], EAX ; xx
	RET
;================== data =====================
segment readable writeable
;=============================================
; symbols: 1000 entries, 11 used
; ------------------------------------
I4			rd 100        ; xx (4)
C5			rb 256        ; y
I6			rd 1          ; a (6)
I7			rd 1          ; a2 (7)
C8			rb 1          ; b
locals		rd 400
