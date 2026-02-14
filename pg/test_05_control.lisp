;; Test 05: Control Flow
;; Testing if and cond statements

;; Basic if statements - true condition
(if #t 'yes 'no)           ; Expected: yes
(if (< 3 5) 'small 'big)   ; Expected: small  
(if (eq? 2 2) 'equal 'different) ; Expected: equal

;; Basic if statements - false condition
(if () 'yes 'no)           ; Expected: no
(if (< 5 3) 'small 'big)   ; Expected: big
(if (eq? 2 3) 'equal 'different) ; Expected: different

;; If with complex conditions
(if (and #t #t) 'both-true 'not-both) ; Expected: both-true
(if (or () #t) 'one-true 'both-false)  ; Expected: one-true
(if (not ()) 'not-false 'false)        ; Expected: not-false

;; If with expressions in branches
(if (< 2 5) (+ 1 1) (* 2 2))           ; Expected: 2
(if (> 2 5) (+ 1 1) (* 2 2))           ; Expected: 4 (note: no > function, so this might error)

;; Nested if statements
(if #t 
    (if #t 'inner-yes 'inner-no) 
    'outer-no)             ; Expected: inner-yes

(if () 
    'outer-yes
    (if #t 'inner-yes 'inner-no)) ; Expected: inner-yes

;; Basic cond statements
(cond (#t 'first))         ; Expected: first
(cond (() 'first) 
      (#t 'second))        ; Expected: second

;; Multiple cond clauses
(cond ((< 5 3) 'small)
      ((< 5 10) 'medium) 
      (#t 'large))         ; Expected: medium

(cond ((eq? 1 2) 'one)
      ((eq? 2 3) 'two)
      ((eq? 3 3) 'three)
      (#t 'other))         ; Expected: three

;; Cond with expressions in conditions
(cond ((not ()) 'not-nil)
      (#t 'default))       ; Expected: not-nil

(cond ((and () #t) 'and-true)
      ((or #t ()) 'or-true)
      (#t 'default))       ; Expected: or-true

;; Cond with expressions in results  
(cond ((< 2 5) (+ 10 5))
      (#t (* 2 3)))        ; Expected: 15

(cond (() (* 2 2))
      (#t (+ 3 4)))        ; Expected: 7

;; Nested cond statements
(cond (#t 
       (cond ((< 1 2) 'inner-true)
             (#t 'inner-default)))) ; Expected: inner-true

;; Complex control flow
(if (cond ((< 2 1) ())
          ((< 1 2) #t)
          (#t ())) 
    'complex-true 
    'complex-false)        ; Expected: complex-true

;; Using control flow with variables (when defined)
;; Note: These will error until variables are defined
;; (if (< x 10) 'small 'big)
;; (cond ((eq? status 'ready) 'go) (#t 'wait))