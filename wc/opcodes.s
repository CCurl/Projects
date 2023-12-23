_nop:       NEXT

_bye:       xor rdi, rdi    ; exit code 0
            mov rax, 60     ; sys_exit
            syscall
            ret

_stop:      ; pop rax
            ret
            ;NEXT

_lit:       nextCell rax
            dPUSH rax
            NEXT

_exit:      cmp RSPTR, rstk
            jg .1
            mov RSPTR, rstk
            ret
.1:         rPOP PC
            NEXT

_call:      nextCell rax
            rPUSH PC
            mov PC, rax
            NEXT

_jmp:       nextCell rax
            mov PC, rax
            NEXT

_jmpz:      dPOP rax
            test rax, rax
            jz _jmp
            add PC, CELL_SZ
            NEXT

_jmpnz:     dPOP rax
            test rax, rax
            jnz _jmp
            add PC, CELL_SZ
            NEXT

_store:     dPOP rsi
            dPOP rax
            mov [rsi], rax
            NEXT

_cstore:    dPOP rsi
            dPOP rax
            mov [rsi], al
            NEXT

_fetch:     mov TOS, qword [TOS]
            NEXT

_cfetch:    movzx TOS, byte [TOS]
            NEXT

_dup:       dPUSH TOS
            NEXT

_swap:      dPOP rax
            dPOP rbx
            dPUSH rax
            dPUSH rbx
            NEXT

_over:      dPOP rax
            mov rbx, TOS
            dPUSH rax
            dPUSH rbx
            NEXT

_drop:      dPOP rax
            NEXT

_add:       dPOP rax
            add TOS, rax
            NEXT

_mult:      dPOP rbx
            imul TOS, rbx
            NEXT

_slmod:     dPOP rbx
            dPOP rax
            xor rdx, rdx
            idiv rbx
            dPUSH rdx
            dPUSH rax
            NEXT

_sub:       dPOP rax
            sub TOS, rax
            NEXT

_inc:       inc TOS
            NEXT

_dec:       dec TOS
            NEXT

_t:         mov TOS, 1
            NEXT

_f:         mov TOS, 0
            NEXT

_lt:        dPOP rax
            cmp TOS, rax
            jl _t
            jmp _f

_eq:        dPOP rax
            cmp TOS, rax
            je _t
            jmp _f

_gt:        dPOP rax
            cmp TOS, rax
            jg _t
            jmp _f

_eq0:       test TOS, TOS
            jz _t
            jmp _f

_rto:       dPOP rax
            rPUSH rax
            NEXT

_rfetch:    dPUSH [RSPTR]
            NEXT

_rfrom:     rPOP rax
            dPUSH rax
            NEXT

_do:        mov  r9, [lsp]
            add  r9, CELL_SZ*3
            mov  [lsp], r9
            mov  [r9-CELL_SZ*2], r11    ; Save Current Index to lstk[lsp-2]
            dPOP r11                    ; r11 = Index
            dPOP [r9]                   ; lstk[lsp] = Upper Bound
            mov [r9-CELL_SZ], PC        ; lstk[lsp-1] = PC
            NEXT

_unloop:    mov r9, [lsp]
            mov r11, [r9-CELL_SZ*2]     ; Restore saved index
            sub r9, CELL_SZ*3
            cmp r9, lstk
            jge .1
            mov r9, lstk
.1:         mov [lsp], r9
            NEXT

_loop:      mov r9, [lsp]
            inc r11
            cmp r11, [r9]
            jge _unloop
            mov PC, [r9-CELL_SZ]
            NEXT

_loop2:     mov r9, [lsp]
            dec r11
            cmp r11, [r9]
            jle _unloop
            mov PC, [r9-CELL_SZ]
            NEXT

_index:     dPUSH r11
            NEXT

_com:       not TOS
            NEXT

_and:       dPOP rax
            and TOS, rax
            NEXT

_or:        dPOP rax
            or TOS, rax
            NEXT

_xor:       dPOP rax
            xor TOS, rax
            NEXT

; ( a n-- )
_type:      dPOP rdx        ; len
            dPOP rsi        ; string
            call stype
            NEXT

_ztype:     dPOP rsi        ; string
            call strlen
            call stype
            NEXT

_reg_i:     nextCell rax    ; reg number
			mov rdx, [regBase]
			inc qword [rdx+rax*8]
            NEXT

_reg_d:     nextCell rax    ; reg number
			mov rdx, [regBase]
			dec qword [rdx+rax*8]
			NEXT

_reg_r:     nextCell rax    ; reg number
			mov rdx, [regBase]
			mov rbx, [rdx+rax*8]
			dPUSH rbx
            NEXT

_reg_rd:    nextCell rax    ; reg number
			mov rdx, [regBase]
			mov rbx, [rdx+rax*8]
			dPUSH rbx
			dec qword [rdx+rax*8]
            NEXT

_reg_ri:    nextCell rax    ; reg number
			mov rdx, [regBase]
			mov rbx, [rdx+rax*8]
			dPUSH rbx
			inc qword [rdx+rax*8]
            NEXT

_reg_s:     nextCell rax    ; reg number
			mov rdx, [regBase]
			mov rbx, [rdx+rax*8]
			dPOP rbx
			mov [rdx+rax*8], rbx
            NEXT

_reg_new:   add qword [regBase], 10*CELL_SZ
            NEXT

_reg_free:  sub qword [regBase], 10*CELL_SZ
            NEXT

; ------------------------------------------------------------------------------
; SYS OPS
; ------------------------------------------------------------------------------
_sys_ops:    ; xxx TOS
_inline:    ; xxx TOS
            NEXT

_immediate: ; xxx TOS
            NEXT

; ( n-- )
_dot:       dPOP rax
            mov rbx, [base]
            call iToA
            call stype
            NEXT

; ( n -- a )
_itoa:      mov rax, TOS
            mov rbx, [base]
            call iToA
            mov TOS, rsi
            NEXT

; ( a -- n 1 | 0 )
_atoi:      dPOP rsi
            call isNum
            dPUSH rax
            NEXT

_colondef:  ; xxx TOS
            NEXT

_endword:   ; xxx TOS
            NEXT

_create:    ; xxx TOS
            NEXT

;( a1 n-- a2 | 0 )
_find:      dPOP rdx
            dPOP rsi
            call findInDict
            dPUSH rbx
            NEXT

;( -- a n )
_word:      call nextWord
            dPUSH rsi
            dPUSH rdx
            NEXT

_timer:     ; xxx TOS
            NEXT

_ccomma:    dPOP rax
            CComma al
            NEXT

_comma:     dPOP rax
            Comma rax
            NEXT

_key:       ; xxx TOS
            NEXT

_qkey:      ; xxx TOS
            NEXT

_emit:      dPOP rax
            call semit
            NEXT

; ( a-- )
_qtype:     dPOP rsi
            call strlen
            call stype
            NEXT

; ( a1 n1--a2 n2 )
_read:      dPOP rdx
            dPOP rsi
            call readLine
            push rsi
            push rax
            NEXT

; ------------------------------------------------------------------------------
; STRING OPS
; ------------------------------------------------------------------------------
_trunc:     dPOP rsi
            mov [rsi], word 0
            NEXT

_lcase:     ; xxx TOS
            NEXT

_ucase:     ; xxx TOS
            NEXT

_strcpy:    ; xxx TOS
            NEXT

_strcat:    ; xxx TOS
            NEXT

_strcatc:   ; xxx TOS
            NEXT

_strlen:    dPOP rsi
            call strlen
            dPUSH rdx
            NEXT

_streq:     ; xxx TOS
            NEXT

_streqi:    ; xxx TOS
            NEXT

_ltrim:     ; xxx TOS
            NEXT

_rtrim:     ; xxx TOS
            NEXT

; ------------------------------------------------------------------------------
; FLOAT OPS
; ------------------------------------------------------------------------------
_fadd:      ; xxx TOS
            NEXT

_fsub:      ; xxx TOS
            NEXT

_fmul:      ; xxx TOS
            NEXT

_fdiv:      ; xxx TOS
            NEXT

_feq:       ; xxx TOS
            NEXT

_flt:       ; xxx TOS
            NEXT

_fgt:       ; xxx TOS
            NEXT

_f2i:       ; xxx TOS
            NEXT

_i2f:       ; xxx TOS
            NEXT

_fdot:      ; xxx TOS
            NEXT

_sqrt:      ; xxx TOS
            NEXT

_tanh:      ; xxx TOS
            NEXT
