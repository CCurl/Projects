;; Test 06: Variables and Scoping
;; Testing define and let* operations

;; Basic define operations
(define x 42)              ; Expected: x
x                          ; Expected: 42

(define hello 'world)      ; Expected: hello  
hello                      ; Expected: world

(define pi 3.14159)        ; Expected: pi
pi                         ; Expected: 3.14159

;; Define with expressions
(define sum (+ 1 2 3))     ; Expected: sum
sum                        ; Expected: 6

(define nested (cons 1 '(2 3))) ; Expected: nested
nested                     ; Expected: (1 2 3)

;; Redefining variables
(define y 10)              ; Expected: y
y                          ; Expected: 10
(define y 20)              ; Expected: y  
y                          ; Expected: 20

;; Using defined variables in expressions
(define a 5)               ; Expected: a
(define b 10)              ; Expected: b
(+ a b)                    ; Expected: 15
(* a b)                    ; Expected: 50
(< a b)                    ; Expected: #t

;; Basic let* operations
(let* ((x 5)) x)           ; Expected: 5
(let* ((name 'alice)) name) ; Expected: alice

;; Let* with multiple bindings
(let* ((x 5) (y 10)) (+ x y))    ; Expected: 15
(let* ((a 2) (b 3)) (* a b))     ; Expected: 6

;; Let* with sequential dependencies  
(let* ((x 5) (y (* x 2))) y)     ; Expected: 10
(let* ((a 3) (b (+ a 2)) (c (* a b))) c) ; Expected: 15

;; Let* scoping (local vs global)
(define outer 100)         ; Expected: outer  
outer                      ; Expected: 100
(let* ((outer 200)) outer) ; Expected: 200
outer                      ; Expected: 100 (unchanged)

;; Nested let* expressions
(let* ((x 1) (y 2)) 
  (let* ((z 3)) (+ x y z))) ; Expected: 6

;; Let* with complex expressions
(let* ((lst '(1 2 3))
       (first (car lst))
       (rest (cdr lst)))
  (cons first rest))       ; Expected: (1 2 3)

;; Let* accessing globals  
(define global-val 42)     ; Expected: global-val
(let* ((local-val 10)) 
  (+ global-val local-val)) ; Expected: 52

;; Define vs let* scoping test
(define scope-test 'global) ; Expected: scope-test
(let* ((scope-test 'local)
       (result scope-test)) 
  result)                  ; Expected: local
scope-test                 ; Expected: global

;; Complex scoping with list operations
(define my-list '(a b c))  ; Expected: my-list
(let* ((first (car my-list))
       (second (car (cdr my-list))))
  (cons second (cons first '()))) ; Expected: (b a)

;; Let* with conditional expressions
(let* ((x 10)
       (result (if (< x 15) 'small 'big)))
  result)                  ; Expected: small

;; Multiple sequential let* bindings
(let* ((a 1)) 
  (let* ((b 2))
    (let* ((c 3))
      (+ a b c))))         ; Expected: 6