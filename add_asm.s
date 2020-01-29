 .include "xc.inc"

    .text

    ; variables and functions that are to be used in the C
    ;   domain must be declared 'global'.  Additionally, an
    ;   underscore must be placed in front of it since the 
    ;   compiler places an underscore in front of code to be
    ;   assembled.
    .global _add

_add:
    add     w0, w1, w2
    btsc    SR, #2  ; check the overflow bit
    goto    _add_sat
    mov     w2, w0

    return

    ; if the code gets here, then an overflow occured, need to saturate
    ; mask = 0x8000
_add_sat:
    ; if w0 is negative, then saturate negative, otherwise positive
    btsc    w0, #15
    goto    _add_neg_sat

_add_pos_sat:
    mov     #32767, w0
    return

_add_neg_sat:
    mov     #32768, w0
    return

    .end

