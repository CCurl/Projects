;; Test 09: Error Cases - SAFE VERSION
;; Testing various error conditions that should return ERR

;; Undefined variables
hello                      ; Expected: ERR
undefined-var              ; Expected: ERR

;; Type errors - car/cdr on non-pairs
(car 42)                   ; Expected: ERR
(car 'atom)                ; Expected: ERR  
(cdr 42)                   ; Expected: ERR
(cdr 'atom)                ; Expected: ERR

;; Car/cdr on empty list
(car '())                  ; Expected: ERR
(cdr '())                  ; Expected: ERR

;; Invalid function calls
(42 1 2)                   ; Expected: ERR (42 is not a function)

;; Mixed type operations that might error
(+ 'hello 'world)          ; Expected: ERR or NaN  
(* 'atom 5)                ; Expected: ERR or NaN

;; Nested errors
(car (cdr 42))             ; Expected: ERR (cdr 42 should error first)
(+ (car 42) 5)             ; Expected: ERR

;; Testing error recovery - can interpreter continue after errors?
(car 42)                   ; Expected: ERR
(+ 1 2)                    ; Expected: 3 (should work after error)
(cdr 'hello)               ; Expected: ERR
(* 3 4)                    ; Expected: 12 (should work after error)

;; Complex error cases
(define error-test (lambda (x)
  (if (eq? x 'error)
      (car 42)             ; Error case
      x)))                 ; Expected: error-test
(error-test 'hello)        ; Expected: hello
(error-test 'error)        ; Expected: ERR
(error-test 'world)        ; Expected: world

;; Verify interpreter state after errors
(define recovery-test 'ok) ; Expected: recovery-test
recovery-test              ; Expected: ok (should work after errors above)

;; Test completion marker
'error-test-complete       ; Expected: error-test-complete