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

_store:     ; xxx TOS
            jmp next

_cstore:    ; xxx TOS
            jmp next

_fetch:     ; xxx TOS
            jmp next

_cfetch:    ; xxx TOS
            jmp next

_dup:       ; xxx TOS
            jmp next

_swap:      ; xxx TOS
            jmp next

_over:      ; xxx TOS
            jmp next

_drop:      ; xxx TOS
            jmp next

_add:       dPOP rax
            add TOS, rax
            jmp next

_mult:      ; xxx TOS
            jmp next

_slmod:     ; xxx TOS
            jmp next

_sub:       dPOP rax
            sub TOS, rax
            jmp next

_inc:       ; xxx TOS
            jmp next

_dec:       ; xxx TOS
            jmp next

_lt:        ; xxx TOS
            jmp next

_eq:        ; xxx TOS
            jmp next

_gt:        ; xxx TOS
            jmp next

_eq0:       ; xxx TOS
            jmp next

_rto:       ; xxx TOS
            jmp next

_rfetch:    ; xxx TOS
            jmp next

_rfrom:     ; xxx TOS
            jmp next

_do:        ; xxx TOS
            jmp next

_loop:      ; xxx TOS
            jmp next

_loop2:     ; xxx TOS
            jmp next

_index:     ; xxx TOS
            jmp next

_com:       ; xxx TOS
            jmp next

_and:       ; xxx TOS
            jmp next

_or:        ; xxx TOS
            jmp next

_xor:       ; xxx TOS
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
