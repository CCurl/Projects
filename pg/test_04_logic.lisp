;; Test 04: Logic Operations  
;; Testing not, and, or, eq? operations

;; Basic not operations
(not #t)                   ; Expected: ()
(not ())                   ; Expected: #t
(not 42)                   ; Expected: ()
(not 'hello)               ; Expected: ERR or ()

;; Basic and operations  
(and)                      ; Expected: #t (no arguments)
(and #t)                   ; Expected: #t
(and ())                   ; Expected: ()
(and #t #t)                ; Expected: #t
(and #t ())                ; Expected: ()
(and () #t)                ; Expected: ()

;; Multiple and operations
(and #t #t #t)             ; Expected: #t  
(and #t #t ())             ; Expected: ()
(and () #t #t)             ; Expected: ()

;; Basic or operations
(or)                       ; Expected: () or #t (implementation dependent) 
(or #t)                    ; Expected: #t
(or ())                    ; Expected: ()
(or #t ())                 ; Expected: #t  
(or () #t)                 ; Expected: #t
(or () ())                 ; Expected: ()

;; Multiple or operations  
(or () () #t)              ; Expected: #t
(or #t () ())              ; Expected: #t
(or () () ())              ; Expected: ()

;; Short-circuit evaluation for and
(and () (/ 1 0))          ; Expected: () (should not evaluate division)
(and #t 42)               ; Expected: 42 (returns last truthy value)

;; Short-circuit evaluation for or  
(or #t (/ 1 0))           ; Expected: #t (should not evaluate division)
(or () 42)                ; Expected: 42 (returns first truthy value)

;; Equality testing
(eq? 5 5)                  ; Expected: #t
(eq? 5 6)                  ; Expected: ()
(eq? 'hello 'hello)        ; Expected: #t  
(eq? 'hello 'world)        ; Expected: ()
(eq? #t #t)                ; Expected: #t
(eq? () ())                ; Expected: #t

;; List equality (probably fails - tests identity, not structure)
(eq? '(1 2) '(1 2))        ; Expected: () (different identity)

;; Combining logical operations
(and (not ()) (eq? 5 5))   ; Expected: #t
(or (eq? 1 2) (< 3 5))     ; Expected: #t  
(not (and () #t))          ; Expected: #t
(not (or () ()))           ; Expected: #t

;; Complex logical expressions
(and (< 2 5) (< 5 10) (eq? 3 3))      ; Expected: #t
(or (eq? 1 2) (eq? 3 4) (eq? 5 5))    ; Expected: #t
(and (not ()) (or #t ()) (eq? 7 7))   ; Expected: #t

;; Boolean values with arithmetic
(eq? (+ 2 3) 5)            ; Expected: #t
(< (+ 1 2) (* 2 3))        ; Expected: #t
(and (< 1 5) (eq? (* 2 2) 4)) ; Expected: #t