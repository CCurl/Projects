_nop:       ; xxx TOS
            NEXT

_stop:      xor rdi, rdi    ; exit code 0
            mov rax, 60     ; sys_exit
            syscall
            ret

_lit1:      nextByte rax
            dPUSH rax
            NEXT

_lit:       nextCell rax
            dPUSH rax
            NEXT

_exit:      rPOP PC
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
            add PC, 8
            NEXT

_jmpnz:     dPOP rax
            test rax, rax
            jnz _jmp
            add PC, 8
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

_slmod:     ; xxx TOS
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

_rfetch:    dPUSH [RSP]
            NEXT

_rfrom:     rPOP rax
            dPUSH rax
            NEXT

_do:        mov  rsi, [lsp]
            add  rsi, 24
            mov  [lsp], rsi
            dPOP [rsi]              ; lstk[lsp] = Index
            dPOP [rsi-8]            ; lstk[lsp-2] = Upper Bound
            mov  [rsi-16], PC       ; lstk[lsp-2] = PC
            jmp  next

_unloop:    mov rsi, [lsp]
            sub rsi, 24
            cmp rsi, lstk
            jge .1
            mov rsi, lstk
.1:         mov [lsp], rsi
            NEXT

_loop:      mov rsi, [lsp]
            mov rax, [rsi]
            inc rax
            cmp rax, [rsi-8]
            jge _unloop
            mov [rsi], rax
            mov PC, [rsi-16]
            NEXT

_loop2:     mov rsi, [lsp]
            mov rax, [rsi]
            dec rax
            cmp rax, [rsi-8]
            jle _unloop
            mov [rsi], rax
            mov PC, [rsi-16]
            NEXT

_index:     mov rax, [lsp]
            dPUSH rax
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

_type:      dPOP rdx        ; len
            dPOP rsi        ; string
            call stype
            NEXT

_ztype:     dPOP rsi        ; string
            call strlen
            call stype
            NEXT

_reg_i:     ; xxx TOS
            NEXT

_reg_d:     ; xxx TOS
            NEXT

_reg_r:     ; xxx TOS
            NEXT

_reg_rd:    ; xxx TOS
            NEXT

_reg_ri:    ; xxx TOS
            NEXT

_reg_s:     ; xxx TOS
            NEXT

_reg_new:   ; xxx TOS
            NEXT

_reg_free:  ; xxx TOS
            NEXT

; ------------------------------------------------------------------------------
; SYS OPS
; ------------------------------------------------------------------------------
_sys_ops:   nextByte rcx
            mov rax, [sys_ops+rcx*8]
            jmp rax

_inline:    ; xxx TOS
            NEXT

_immediate: ; xxx TOS
            NEXT

_dot:       ; xxx TOS
            NEXT

_itoa:      ; xxx TOS
            NEXT

_atoi:      ; xxx TOS
            NEXT

_colondef:  ; xxx TOS
            NEXT

_endword:   ; xxx TOS
            NEXT

_create:    ; xxx TOS
            NEXT

_find:      ; xxx TOS
            NEXT

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

_qtype:     dPOP rsi
            call strlen
            call stype
            NEXT

_read:      dPOP rdx
            dPOP rsi
            call readLine
            push rsi
            push rax
            NEXT

; ------------------------------------------------------------------------------
; STRING OPS
; ------------------------------------------------------------------------------
_str_ops:   nextByte rcx
            mov rax, [str_ops+rcx*8]
            jmp rax

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
_flt_ops:   nextByte rcx
            mov rax, [flt_ops+rcx*8]
            jmp rax

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
