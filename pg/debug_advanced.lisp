;; Test the problematic parts individually

;; Church numerals test
(define two (lambda (f) (lambda (x) (f (f x)))))
(define church-to-num (lambda (church-num)
  ((church-num (lambda (n) (+ n 1))) 0)))
(church-to-num two)

;; Complex eval test
(eval '(let* ((x 5) (y (* x 2))) (+ x y)))

;; Polynomial test
(define make-polynomial (lambda (coeffs)
  (lambda (x)
    (+ (* (car coeffs) x) (car (cdr coeffs))))))
(define linear (make-polynomial '(2 3)))
(linear 5)