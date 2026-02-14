;; Test just the polynomial part
(define make-polynomial (lambda (coeffs)
  (lambda (x)
    (+ (* (car coeffs) x) (car (cdr coeffs))))))
(define linear (make-polynomial '(2 3)))
linear
(linear 5)