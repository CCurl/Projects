; 6 instructions removed
format ELF executable
;================== code =====================
segment readable executable
start:
	LEA  EBP, [locals]
	CALL F11
;================== library ==================
exit:
	MOV EAX, 1
	XOR EBX, EBX
	INT 0x80

;=============================================

F9: ; func
	MOV  EAX, [EBP+8]
	IMUL EAX, 3
	ADD  EAX, [EBP+4]
	MOV  [EBP+12], EAX
	RET

F10: ; pushA
	MOV  EAX, [I4] ; stk
	ADD  EAX, 1
	MOV  [I4], EAX ; stk
	MOV  EAX, [I4] ; stk
	PUSH EAX
	MOV  EAX, [I6] ; a
	POP  ECX
	MOV  [I4 + ECX*4], EAX ; stk
	RET

F11: ; main
	ADD  EBP, 40
	MOV  EAX, 4
	MOV  [EBP+4], EAX
	MOV  EAX, 5
	MOV  [EBP+8], EAX
	CALL F9 ; func
	MOV  EAX, [EBP+12]
	MOV  [I6], EAX ; a
	SUB  EBP, 40
	MOV  EAX, [I6] ; a
	MOV  [C8], AL ; b
	MOV  EAX, [I6] ; a
	INC  [I6] ; a
	PUSH EAX
	MOV  EAX, 97
	POP  ECX
	MOV  [C5 + ECX], AL ; y
	MOV  EAX, 2
	PUSH EAX
	MOV  EAX, [I6] ; a
	ADD  EAX, 223
	POP  ECX
	MOV  [C5 + ECX], AL ; y
	MOV  EAX, [I6] ; a
	PUSH EAX
	MOV  EAX, 123
	POP  ECX
	MOV  [C5 + ECX], AL ; y
	MOV  EAX, [I6] ; a
	ADD  EAX, 1
	PUSH EAX
	MOV  EAX, 0
	POP  ECX
	MOV  [C5 + ECX], AL ; y
	CALL F10 ; pushA
	MOV  EAX, [I6] ; a
	ADD  EAX, 1
	MOV  [I6], EAX ; a
	CALL F10 ; pushA
	RET
;================== data =====================
segment readable writeable
;=============================================
; symbols: 1000 entries, 12 used
; ------------------------------------
I4			rd 100        ; stk (4)
C5			rb 256        ; y
I6			rd 1          ; a (6)
I7			rd 1          ; a2 (7)
C8			rb 1          ; b
locals		rd 400
