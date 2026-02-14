;; Test 08: Advanced Features - SAFE VERSION
;; Testing eval, quote, meta-programming, simplified algorithms

;; Meta-programming with eval and quote
(eval '(+ 1 2))            ; Expected: 3
(eval '(* 3 4))            ; Expected: 12
(eval (quote (- 10 3)))    ; Expected: 7

;; Quote vs eval
(quote (+ 1 2))            ; Expected: (+ 1 2)  
'(+ 1 2)                   ; Expected: (+ 1 2)
(eval (quote (+ 1 2)))     ; Expected: 3

;; Dynamic code construction
(define make-expr (lambda (op a b)
  (cons op (cons a (cons b '()))))) ; Expected: make-expr
(make-expr '+ 5 10)        ; Expected: (+ 5 10)
(eval (make-expr '+ 5 10)) ; Expected: 15
(eval (make-expr '* 3 7))  ; Expected: 21

;; Self-evaluating expressions
(eval 42)                  ; Expected: 42
(eval '())                 ; Expected: ()
(eval '#t)                 ; Expected: #t

;; Higher-order functions with eval
(define apply-op (lambda (operation args)
  (eval (cons operation args)))) ; Expected: apply-op
(apply-op '+ '(1 2 3))     ; Expected: 6
(apply-op '* '(2 3 4))     ; Expected: 24

;; Mathematical algorithms - Simple map function
(define map (lambda (f lst)
  (if (pair? lst)
      (cons (f (car lst)) (map f (cdr lst)))
      '())))               ; Expected: map
(map (lambda (x) (* x 2)) '(1 2 3)) ; Expected: (2 4 6)

;; Simple filter function  
(define filter (lambda (pred lst)
  (cond ((not (pair? lst)) '())
        ((pred (car lst)) (cons (car lst) (filter pred (cdr lst))))
        (#t (filter pred (cdr lst)))))) ; Expected: filter
        
(filter (lambda (x) (< x 5)) '(1 6 2 3)) ; Expected: (1 2 3)

;; Simple fold/reduce function
(define fold (lambda (f acc lst)
  (if (pair? lst)
      (fold f (f acc (car lst)) (cdr lst))
      acc)))               ; Expected: fold
(fold + 0 '(1 2 3))        ; Expected: 6

;; Tree operations (simplified)
(define make-node (lambda (val left right)
  (cons val (cons left right)))) ; Expected: make-node
(define node-val (lambda (node) (car node))) ; Expected: node-val
(define tree (make-node 10 5 15)) ; Expected: tree
(node-val tree)            ; Expected: 10

;; Church numerals (basic)
(define zero (lambda (f) (lambda (x) x))) ; Expected: zero
(define succ (lambda (n) (lambda (f) (lambda (x) (f ((n f) x)))))) ; Expected: succ
(define one (succ zero))   ; Expected: one

;; Simple polynomial (safer version)
(define linear (lambda (a b x) (+ (* a x) b))) ; Expected: linear
(linear 2 3 5)             ; Expected: 13 (2*5 + 3)

'advanced-test-complete    ; Expected: advanced-test-complete