;; Test 01: Basic Data Types and Literals
;; Testing numbers, atoms, lists, quotes

;; Numbers
42                          ; Expected: 42
3.14159                     ; Expected: 3.14159
-17                         ; Expected: -17
0                          ; Expected: 0
1e6                        ; Expected: 1000000

;; Atoms/Symbols  
hello                      ; Expected: ERR (undefined variable)
'hello                     ; Expected: hello
'x                         ; Expected: x
'my-variable               ; Expected: my-variable

;; Empty list
()                         ; Expected: ()
'()                        ; Expected: ()

;; Simple lists
'(1 2 3)                   ; Expected: (1 2 3)
'(a b c)                   ; Expected: (a b c)
'(hello world)             ; Expected: (hello world)

;; Mixed type lists
'(1 hello 3.14)           ; Expected: (1 hello 3.14)
'(+ - * /)                ; Expected: (+ - * /)

;; Nested lists
'((1 2) (3 4))            ; Expected: ((1 2) (3 4))
'(a (b c) d)              ; Expected: (a (b c) d)
'(((nested)))             ; Expected: (((nested)))

;; Quote vs unquoted
(quote hello)             ; Expected: hello
(quote (1 2 3))           ; Expected: (1 2 3)

;; Single element lists
'(42)                     ; Expected: (42)
'(single)                 ; Expected: (single)