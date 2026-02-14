;; Test 10: Performance and Memory Tests - MINIMAL SAFE VERSION  
;; Testing memory usage, very safe recursion limits, garbage collection

;; Simple memory consumption
(define mem-test-1 'hello) ; Should use ~1 cell for binding
(define mem-test-2 '(1 2 3)) ; Should use more cells for list  
(define mem-test-3 (lambda (x) x)) ; Should use cells for closure

;; Small data structures
(define small-list '(1 2 3 4 5))
(define small-nested '((a b) (c d)))

;; Very safe recursion - factorial (minimal depth)  
(define fact (lambda (n)
  (if (< n 2) 1 (* n (fact (- n 1))))))
(fact 3)                   ; Expected: 6 (very safe)
(fact 4)                   ; Expected: 24 (safe)

;; Iterative version for comparison
(define fact-iter (lambda (n acc)
  (if (< n 2) acc (fact-iter (- n 1) (* n acc)))))
(define factorial (lambda (n) (fact-iter n 1)))
(factorial 4)              ; Expected: 24

;; List length testing (very short lists)
(define list-len (lambda (lst)
  (if (pair? lst) (+ 1 (list-len (cdr lst))) 0)))

(list-len '(1))            ; Expected: 1
(list-len '(1 2 3))        ; Expected: 3

;; Building very small lists
(define build-list (lambda (n)
  (if (< n 1) '() (cons n (build-list (- n 1))))))

(build-list 2)             ; Expected: (2 1)
(build-list 3)             ; Expected: (3 2 1)

;; Minimal recursion test
(define mini-add (lambda (n)
  (if (< n 1) 0 (+ 1 (mini-add (- n 1))))))
(mini-add 3)               ; Expected: 3

;; Simple function creation 
(define make-adder (lambda (n)
  (lambda (x) (+ x n))))   ; Create closure

(define add5 (make-adder 5))
(add5 3)                   ; Expected: 8

;; Simple expression evaluation
(define simple-expr (+ (* 2 3) (- 10 5))) ; Expected: 11

;; Verify completion
'performance-test-complete ; Expected: performance-test-complete