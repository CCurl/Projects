;; Test 10: Performance and Memory Tests - SAFE VERSION  
;; Testing memory usage, safe recursion limits, garbage collection

;; Memory usage tracking - watch the prompt numbers
;; The prompt shows available memory, e.g., "925>" means 925 cells free

;; Simple memory consumption
(define mem-test-1 'hello) ; Should use ~1 cell for binding
(define mem-test-2 '(1 2 3)) ; Should use more cells for list  
(define mem-test-3 (lambda (x) x)) ; Should use cells for closure

;; Small data structures
(define small-list '(1 2 3 4 5))
(define small-nested '((a b) (c d)))

;; Safe recursion - factorial (safe depth)  
(define fact (lambda (n)
  (if (< n 2) 1 (* n (fact (- n 1))))))
(fact 5)                   ; Expected: 120 (safe)
(fact 8)                   ; Expected: 40320 (should be safe)

;; Iterative version for comparison (more memory efficient)
(define fact-iter (lambda (n acc)
  (if (< n 2) acc (fact-iter (- n 1) (* n acc)))))
(define factorial (lambda (n) (fact-iter n 1)))
(factorial 8)              ; Expected: 40320

;; List length testing (linear recursion - safe)
(define list-len (lambda (lst)
  (if (pair? lst) (+ 1 (list-len (cdr lst))) 0)))

(list-len '(1))            ; Expected: 1
(list-len '(1 2 3 4 5))    ; Expected: 5  

;; Building small lists
(define build-list (lambda (n)
  (if (< n 1) '() (cons n (build-list (- n 1))))))

(build-list 3)             ; Expected: (3 2 1)
(build-list 5)             ; Expected: (5 4 3 2 1)

;; Safe recursion depth test
(define safe-add (lambda (n)
  (if (< n 1) 0 (+ 1 (safe-add (- n 1))))))
(safe-add 5)               ; Expected: 5
(safe-add 10)              ; Expected: 10 (safe depth)

;; Tail recursive version (should be more efficient)
(define tail-add (lambda (n acc)
  (if (< n 1) acc (tail-add (- n 1) (+ acc 1)))))
(define count-to (lambda (n) (tail-add n 0)))
(count-to 10)              ; Expected: 10

;; Small tree structures (safe depth)
(define make-small-tree (lambda (depth)
  (if (< depth 1) 'leaf
      (cons 'node (make-small-tree (- depth 1))))))

(make-small-tree 1)        ; Expected: (node . leaf)
(make-small-tree 2)        ; Expected: (node node . leaf)

;; Simple function creation 
(define make-adder (lambda (n)
  (lambda (x) (+ x n))))   ; Create closure

(define add5 (make-adder 5))
(add5 3)                   ; Expected: 8

;; Moderate computation without much memory use
(define safe-computation (lambda (n result)
  (if (< n 1) result
      (safe-computation (- n 1) (+ result 1)))))
(safe-computation 15 0)    ; Expected: 15 (counts to 15)

;; Complex expression evaluation (safe)
(define complex-expr 
  (+ (* 2 3) 
     (- 10 5) 
     (if (< 2 3) 1 0)))    ; Expected: 12 (6+5+1)

;; Verify memory state after tests
'performance-test-complete ; Expected: performance-test-complete