;; Test 03: List Operations
;; Testing cons, car, cdr, pair? operations

;; Basic cons operations
(cons 1 '())               ; Expected: (1)
(cons 1 '(2))              ; Expected: (1 2)
(cons 1 '(2 3))            ; Expected: (1 2 3)
(cons 'a '(b c))           ; Expected: (a b c)

;; Cons with atoms (dotted pairs)
(cons 1 2)                 ; Expected: (1 . 2)
(cons 'a 'b)               ; Expected: (a . b)

;; Basic car operations
(car '(1 2 3))             ; Expected: 1
(car '(a b c))             ; Expected: a
(car '(hello world))       ; Expected: hello
(car '((1 2) 3))           ; Expected: (1 2)

;; Basic cdr operations
(cdr '(1 2 3))             ; Expected: (2 3)
(cdr '(a b c))             ; Expected: (b c)
(cdr '(hello world))       ; Expected: (world)
(cdr '(1))                 ; Expected: ()

;; Car of cdr combinations
(car (cdr '(1 2 3)))       ; Expected: 2
(cdr (cdr '(1 2 3)))       ; Expected: (3)
(car (car '((a b) c)))     ; Expected: a
(cdr (car '((a b) c)))     ; Expected: (b)

;; Pair testing
(pair? '(1 2))             ; Expected: #t
(pair? '())                ; Expected: ()  
(pair? '(single))          ; Expected: #t
(pair? 42)                 ; Expected: ()
(pair? 'atom)              ; Expected: ERR (undefined) or ()

;; Complex list construction
(cons 1 (cons 2 (cons 3 '())))  ; Expected: (1 2 3)
(cons (cons 1 2) 3)             ; Expected: ((1 . 2) . 3)

;; Nested list access
(car (cdr (car '(((a b) c) d)))) ; Expected: c
(cdr (car (cdr '(1 (2 3) 4))))   ; Expected: (3)

;; Building lists step by step
(cons 'x '())              ; Expected: (x)
(cons 'w (cons 'x '()))    ; Expected: (w x)
(cons 'v (cons 'w (cons 'x '()))) ; Expected: (v w x)

;; List deconstruction
(car '((first) second))    ; Expected: (first)
(cdr '((first) second))    ; Expected: (second)
(car (car '((a b) (c d)))) ; Expected: a
(cdr (car '((a b) (c d)))) ; Expected: (b)

;; Edge cases for car/cdr
(car (cons 1 2))           ; Expected: 1
(cdr (cons 1 2))           ; Expected: 2