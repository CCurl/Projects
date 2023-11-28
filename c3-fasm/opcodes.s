_stop:      ; xxx TOS
            jmp next

_lit1:      ; xxx TOS
            jmp next

_lit:       ; xxx TOS
            jmp next

_exit:      ; xxx TOS
            jmp next

_call:      ; xxx TOS
            jmp next

_jmp:       ; xxx TOS
            jmp next

_jmpz:      ; xxx TOS
            jmp next

_jmpnz:     ; xxx TOS
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

_add:       ; xxx TOS
            jmp next

_mult:      ; xxx TOS
            jmp next

_slmod:     ; xxx TOS
            jmp next

_sub:       ; xxx TOS
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

_type:      ; xxx TOS
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

_sys_ops:   ; xxx TOS
            jmp next

_str_ops:   ; xxx TOS
            jmp next

_flt_ops:   ; xxx TOS
            jmp next

_xx-sys-ops:; xxx TOS
            jmp next

_inline:    ; xxx TOS
            jmp next

_immediate: ; xxx TOS
            jmp next

_dot:       ; xxx TOS
            jmp next

_x3:        ; xxx TOS
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

_emit:      ; xxx TOS
            jmp next

_qtype:     ; xxx TOS
            jmp next

_xx-str-ops:; xxx TOS
            jmp next

_trunc:     ; xxx TOS
            jmp next

_lcase:     ; xxx TOS
            jmp next

_ucase:     ; xxx TOS
            jmp next

_x3:        ; xxx TOS
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

_flt-ops:   ; xxx TOS
            jmp next

_fadd:      ; xxx TOS
            jmp next

_fsub:      ; xxx TOS
            jmp next

_fmul:      ; xxx TOS
            jmp next

_x3:        ; xxx TOS
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

_stop, _lit1, _lit, _exit, _call, _jmp, _jmpz, _jmpnz, _store, _cstore, 
_fetch, _cfetch, _dup, _swap, _over, _drop, _add, _mult, _slmod, _sub,
_inc, _dec, _lt, _eq, _gt, _eq0, _rto, _rfetch, _rfrom, _do, 
_loop, _loop2, _index, _com, _and, _or, _xor, _type, _ztype, _reg_i, 
_reg_d, _reg_r, _reg_rd, _reg_ri, _reg_s, _reg_new, _reg_free,
_sys_ops, _str_ops, _flt_ops, 

_xx-sys-ops, 
_inline, _immediate, _dot, _x3, _itoa, _atoi, _colondef, _endword, 
_create, _find, _word, _timer, _ccomma, _comma, _key, _qkey, 
_emit, _qtype, 

_xx-str-ops, 
_trunc, _lcase, _ucase, _x3, _strcpy, _strcat, _strcatc, _strlen, 
_streq, _streqi, _ltrim, _rtrim, 

_flt-ops, 
_fadd, _fsub, _fmul, _x3, _fdiv, _feq, _flt, _fgt, 
_f2i, _i2f, _fdot, _sqrt, _tanh, 
