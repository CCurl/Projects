;; Test 02: Arithmetic Operations
;; Testing +, -, *, /, int, < operators

;; Basic addition
(+ 1 2)                    ; Expected: 3
(+ 0 5)                    ; Expected: 5
(+ -3 7)                   ; Expected: 4

;; Multiple operand addition
(+ 1 2 3)                  ; Expected: 6
(+ 1 2 3 4 5)              ; Expected: 15
(+ 10 20 30 40)            ; Expected: 100

;; Subtraction  
(- 10 5)                   ; Expected: 5
(- 0 3)                    ; Expected: -3
(- 100 25)                 ; Expected: 75

;; Multiple operand subtraction
(- 20 5 3)                 ; Expected: 12
(- 100 20 30 10)           ; Expected: 40

;; Multiplication
(* 3 4)                    ; Expected: 12
(* 0 5)                    ; Expected: 0
(* -2 6)                   ; Expected: -12

;; Multiple operand multiplication  
(* 2 3 4)                  ; Expected: 24
(* 1 2 3 4 5)              ; Expected: 120

;; Division
(/ 12 3)                   ; Expected: 4
(/ 1 2)                    ; Expected: 0.5
(/ 100 10)                 ; Expected: 10

;; Multiple operand division
(/ 24 2 3)                 ; Expected: 4
(/ 120 2 3 4)              ; Expected: 5

;; Decimals in arithmetic
(+ 1.5 2.5)                ; Expected: 4
(* 3.14159 2)              ; Expected: 6.28318

;; Mixed integer/decimal
(+ 1 2.5)                  ; Expected: 3.5
(* 2 3.14159)              ; Expected: 6.28318

;; Integer conversion
(int 3.14159)              ; Expected: 3
(int 7.9)                  ; Expected: 7
(int -2.7)                 ; Expected: -2
(int 42)                   ; Expected: 42

;; Less than comparison
(< 3 5)                    ; Expected: #t
(< 10 5)                   ; Expected: ()
(< 5 5)                    ; Expected: ()
(< -3 0)                   ; Expected: #t
(< 0 -3)                   ; Expected: ()

;; Nested arithmetic
(+ (* 2 3) (/ 8 2))        ; Expected: 10
(- (+ 10 5) (* 2 3))       ; Expected: 9
(* (+ 1 2) (- 5 2))        ; Expected: 9