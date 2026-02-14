;; Test 07: Functions
;; Testing lambda expressions, function application, recursion

;; Basic lambda expressions
(lambda (x) x)             ; Expected: {closure}
(lambda (x) (+ x 1))       ; Expected: {closure}
(lambda (x y) (+ x y))     ; Expected: {closure}

;; Simple function application
((lambda (x) x) 42)        ; Expected: 42
((lambda (x) (+ x 1)) 5)   ; Expected: 6
((lambda (x y) (+ x y)) 3 4) ; Expected: 7

;; Functions with complex expressions
((lambda (x) (* x x)) 7)   ; Expected: 49
((lambda (x) (if (< x 0) (- x) x)) -5) ; Expected: 5 (absolute value)

;; Defining named functions
(define square (lambda (x) (* x x))) ; Expected: square
(square 8)                 ; Expected: 64
(square -3)                ; Expected: 9

(define add (lambda (x y) (+ x y))) ; Expected: add
(add 10 20)                ; Expected: 30

;; Functions with multiple parameters
(define multiply3 (lambda (x y z) (* x y z))) ; Expected: multiply3
(multiply3 2 3 4)          ; Expected: 24

;; Functions that return functions (higher-order)
(define make-adder (lambda (n) (lambda (x) (+ x n)))) ; Expected: make-adder
(define add5 (make-adder 5)) ; Expected: add5
(add5 3)                   ; Expected: 8
(add5 10)                  ; Expected: 15

;; Simple recursion - factorial
(define fact (lambda (n) 
  (if (< n 2) 1 (* n (fact (- n 1)))))) ; Expected: fact
(fact 0)                   ; Expected: 1
(fact 1)                   ; Expected: 1  
(fact 5)                   ; Expected: 120

;; Simple recursion - countdown
(define countdown (lambda (n)
  (if (< n 1) 'done (countdown (- n 1))))) ; Expected: countdown
(countdown 3)              ; Expected: done (after recursion)

;; List processing functions
(define list-length (lambda (lst)
  (if (pair? lst) 
      (+ 1 (list-length (cdr lst)))
      0)))                 ; Expected: list-length 
(list-length '())          ; Expected: 0
(list-length '(1))         ; Expected: 1
(list-length '(1 2 3 4))   ; Expected: 4

;; List reversal (simple version)
(define append-element (lambda (lst elem)
  (if (pair? lst)
      (cons (car lst) (append-element (cdr lst) elem))
      (cons elem '())))) ; Expected: append-element

;; Functions with local variables  
(define circle-area (lambda (radius)
  (let* ((pi 3.14159)
         (r-squared (* radius radius)))
    (* pi r-squared))))    ; Expected: circle-area
(circle-area 2)            ; Expected: ~12.566

;; Closures capturing environment  
(define make-counter (lambda (start)
  (lambda () start)))      ; Expected: make-counter (simplified - no mutation)
(define counter5 (make-counter 5)) ; Expected: counter5  
(counter5)                 ; Expected: 5

;; Functions as parameters
(define apply-twice (lambda (f x)
  (f (f x))))              ; Expected: apply-twice
(apply-twice (lambda (n) (* n 2)) 3) ; Expected: 12

;; Mutual recursion (simple case)
(define is-even (lambda (n)
  (if (eq? n 0) #t 
      (if (eq? n 1) () 
          (is-odd (- n 1)))))) ; Expected: is-even (will need is-odd)

(define is-odd (lambda (n) 
  (if (eq? n 0) ()
      (if (eq? n 1) #t
          (is-even (- n 1)))))) ; Expected: is-odd

;; Note: These might not work due to forward reference issues
;; (is-even 4)                ; Expected: #t  
;; (is-odd 4)                 ; Expected: ()

;; Lambda with no parameters
(define get-pi (lambda () 3.14159)) ; Expected: get-pi  
(get-pi)                   ; Expected: 3.14159