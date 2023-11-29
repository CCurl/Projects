_nop:       ; xxx TOS
            jmp next

_stop:      xor rdi, rdi    ; exit code 0
            mov rax, 60     ; sys_exit
            syscall
            ret

_lit1:      nextByte rax
            dPUSH rax
            jmp next

_lit:       nextCell rax
            dPUSH rax
            jmp next

_exit:      rPOP PC
            jmp next

_call:      nextCell rax
            rPUSH PC
            mov PC, rax
            jmp next

_jmp:       nextCell rax
            mov PC, rax
            jmp next

_jmpz:      dPOP rax
            test rax, rax
            jz _jmp
            add PC, 8
            jmp next

_jmpnz:     dPOP rax
            test rax, rax
            jnz _jmp
            add PC, 8
            jmp next

_store:     dPOP rsi
            dPOP rax
            mov [rsi], rax
            jmp next

_cstore:    dPOP rsi
            dPOP rax
            mov [rsi], al
            jmp next

_fetch:     mov TOS, qword [TOS]
            jmp next

_cfetch:    movzx TOS, byte [TOS]
            jmp next

_dup:       dPUSH TOS
            jmp next

_swap:      dPOP rax
            dPOP rbx
            dPUSH rax
            dPUSH rbx
            jmp next

_over:      dPOP rax
            mov rbx, TOS
            dPUSH rax
            dPUSH rbx
            jmp next

_drop:      dPOP rax
            jmp next

_add:       dPOP rax
            add TOS, rax
            jmp next

_mult:      dPOP rbx
            imul TOS, rbx
            jmp next

_slmod:     ; xxx TOS
            jmp next

_sub:       dPOP rax
            sub TOS, rax
            jmp next

_inc:       inc TOS
            jmp next

_dec:       dec TOS
            jmp next

_t:         mov TOS, 1
            jmp next

_f:         mov TOS, 0
            jmp next

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
            jmp next

_rfetch:    dPUSH [RSP]
            jmp next

_rfrom:     rPOP rax
            dPUSH rax
            jmp next

_do:        mov  rsi, [lsp]
            add  rsi, 3
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
            jmp next

_loop:      mov rsi, [lsp]
            mov rax, [rsi]
            inc rax
            cmp rax, [rsi-8]
            jge _unloop
            mov [rsi], rax
            mov PC, [rsi-16]
            jmp next

_loop2:     mov rsi, [lsp]
            mov rax, [rsi]
            dec rax
            cmp rax, [rsi-8]
            jle _unloop
            mov [rsi], rax
            mov PC, [rsi-16]
            jmp next

_index:     mov rax, [lsp]
            dPUSH rax
            jmp next

_com:       not TOS
            jmp next

_and:       dPOP rax
            and TOS, rax
            jmp next

_or:        dPOP rax
            or TOS, rax
            jmp next

_xor:       dPOP rax
            xor TOS, rax
            jmp next

_type:      dPOP rdx        ; len
            dPOP rsi        ; string
            call stype
            jmp next

_ztype:     ; xxx TOS
            jmp next

_reg_i:     ; xxx TOS
            jmp next

_reg_d:     ; xxx TOS
            jmp next

_reg_r:     ; xxx TOS
            jmp next

_reg_rd:    ; xxx TOS
            jmp next

_reg_ri:    ; xxx TOS
            jmp next

_reg_s:     ; xxx TOS
            jmp next

_reg_new:   ; xxx TOS
            jmp next

_reg_free:  ; xxx TOS
            jmp next

; ------------------------------------------------------------------------------
; SYS OPS
; ------------------------------------------------------------------------------
_sys_ops:   nextByte rcx
            mov rax, [sys_ops+rcx*8]
            jmp rax

_inline:    ; xxx TOS
            jmp next

_immediate: ; xxx TOS
            jmp next

_dot:       ; xxx TOS
            jmp next

_itoa:      ; xxx TOS
            jmp next

_atoi:      ; xxx TOS
            jmp next

_colondef:  ; xxx TOS
            jmp next

_endword:   ; xxx TOS
            jmp next

_create:    ; xxx TOS
            jmp next

_find:      ; xxx TOS
            jmp next

_word:      ; xxx TOS
            jmp next

_timer:     ; xxx TOS
            jmp next

_ccomma:    ; xxx TOS
            jmp next

_comma:     ; xxx TOS
            jmp next

_key:       ; xxx TOS
            jmp next

_qkey:      ; xxx TOS
            jmp next

_emit:      dPOP rax
            call semit
            jmp next

_qtype:     ; xxx TOS
            jmp next

; ------------------------------------------------------------------------------
; STRING OPS
; ------------------------------------------------------------------------------
_str_ops:   nextByte rcx
            mov rax, [str_ops+rcx*8]
            jmp rax

_trunc:     ; xxx TOS
            jmp next

_lcase:     ; xxx TOS
            jmp next

_ucase:     ; xxx TOS
            jmp next

_strcpy:    ; xxx TOS
            jmp next

_strcat:    ; xxx TOS
            jmp next

_strcatc:   ; xxx TOS
            jmp next

_strlen:    ; xxx TOS
            jmp next

_streq:     ; xxx TOS
            jmp next

_streqi:    ; xxx TOS
            jmp next

_ltrim:     ; xxx TOS
            jmp next

_rtrim:     ; xxx TOS
            jmp next

; ------------------------------------------------------------------------------
; FLOAT OPS
; ------------------------------------------------------------------------------
_flt_ops:   nextByte rcx
            mov rax, [flt_ops+rcx*8]
            jmp rax

_fadd:      ; xxx TOS
            jmp next

_fsub:      ; xxx TOS
            jmp next

_fmul:      ; xxx TOS
            jmp next

_fdiv:      ; xxx TOS
            jmp next

_feq:       ; xxx TOS
            jmp next

_flt:       ; xxx TOS
            jmp next

_fgt:       ; xxx TOS
            jmp next

_f2i:       ; xxx TOS
            jmp next

_i2f:       ; xxx TOS
            jmp next

_fdot:      ; xxx TOS
            jmp next

_sqrt:      ; xxx TOS
            jmp next

_tanh:      ; xxx TOS
            jmp next
