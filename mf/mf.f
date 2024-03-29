-ML- ;      1  1 -X-  INLINE
-ML- !AC    5  1 -X-  INLINE
-ML- @AC    6  1 -X-  INLINE
-ML- SYS    7  1 -X-  INLINE
-ML- LIT1   8  1 -X-  INLINE
-ML- @A+    9  1 -X-  INLINE
-ML- @A    11  1 -X-  INLINE
-ML- !     12  1 -X-  INLINE
-ML- !A+   13  1 -X-  INLINE
-ML- @     14  1 -X-  INLINE
-ML- !A    15  1 -X-  INLINE
-ML- COM   16  1 -X-  INLINE
-ML- 2*    17  1 -X-  INLINE
-ML- 2/    18  1 -X-  INLINE
-ML- +*    19  1 -X-  INLINE
-ML- XOR   20  1 -X-  INLINE
-ML- AND   21  1 -X-  INLINE
-ML- +     23  1 -X-  INLINE
-ML- R>    24  1 -X-  INLINE
-ML- A     25  1 -X-  INLINE
-ML- AND   25  1 -X-  INLINE
-ML- DUP   26  1 -X-  INLINE
-ML- OVER  27  1 -X-  INLINE
-ML- >R    28  1 -X-  INLINE
-ML- >A    29  1 -X-  INLINE
-ML- DROP  31  1 -X-  INLINE

-ML- : 8 108 7 1 -X- IMMEDIATE

: EMIT   101 SYS ; INLINE
: .D     102 SYS ; INLINE
: .H     103 SYS ; INLINE
: FOPEN  104 SYS ; INLINE
: FCLOSE 105 SYS ; INLINE
: C,     106 SYS ; INLINE
: ,      107 SYS ; INLINE
: CREATE 108 SYS ; INLINE
: FIND   109 SYS ; INLINE
: (H)    110 SYS ; INLINE
: (L)    111 SYS ; INLINE
: (ST)   112 SYS ; INLINE
: CELL   113 SYS ; INLINE
: MEM    114 SYS ; INLINE
: MEM-SZ 115 SYS ; INLINE
: IMM    116 SYS ; INLINE
: INL    117 SYS ; INLINE

: sp   32 EMIT ; INLINE
: bye (ST) >A 999 !A ;
: negate COM 1 + ;
: - negate + ;
: .d .D sp ;
: .h .H sp ;

: H (H) @ ; : L (L) @ ;

H MEM - .d 
MEM MEM-SZ + L - .d
L H - .d
