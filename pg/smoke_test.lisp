;; Quick Smoke Test - Essential functionality verification
;; This test should complete in under 5 seconds

;; Basic literals
42
3.14  
'hello

;; Basic arithmetic
(+ 1 2)
(* 3 4)
(- 10 3)

;; Basic lists  
'(1 2 3)
(cons 1 '(2))
(car '(a b c))  
(cdr '(1 2 3))

;; Variables
(define x 5)
x

;; Simple function
(define square (lambda (n) (* n n)))
(square 4)

;; Control flow
(if (< 2 5) 'yes 'no)

;; Logic
(and #t #t)
(or () #t)
(not ())

;; Recursion (small)
(define fact (lambda (n) (if (< n 2) 1 (* n (fact (- n 1))))))
(fact 4)

;; Meta-programming
(eval '(+ 2 3))

'smoke-test-complete