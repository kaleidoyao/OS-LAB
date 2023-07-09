%define MAX_LEN_INPUT   203
%define MAX_LEN_OPERAND 101
%define SPACE           32    ; ' '
%define NEW_LINE        10    ; '\n'
%define ASCII_ZERO      48    ; '0'

SECTION .data
msg_zero_devide_zero:   db    "Error: the result is NAN!", 0h
msg_divided_by_zero:    db    "Error: the divisor can not be zero!", 0h 

SECTION .bss
input_string:      resb   MAX_LEN_INPUT
dividend:          resb   MAX_LEN_OPERAND
divisor:           resb   MAX_LEN_OPERAND
quotient:          resb   MAX_LEN_OPERAND
remainder:         resb   MAX_LEN_OPERAND
special_signal:    resb   1
is_enough_flag:    resb   1

SECTION .text
global main

main:
    mov    eax,    input_string
    mov    ebx,    MAX_LEN_INPUT
    call   string_read

    mov    eax,    input_string     ; get dividend
    mov    ebx,    dividend
    call   parse_input
       
    mov    ebx,    divisor          ; get divisor
    call   parse_input

    mov    dl,     BYTE[dividend]   ; judge 0 / 0
    cmp    dl,     ASCII_ZERO
    jne    .is_divided_by_zero
    mov    dl,     BYTE[divisor]
    cmp    dl,     ASCII_ZERO
    je     zero_devide_zero


.is_divided_by_zero:
    mov    dl,     BYTE[divisor]    ; judge if the divisor is 0
    cmp    dl,     ASCII_ZERO
    je     divide_by_zero

    call   is_enough
    cmp    BYTE[is_enough_flag],    0
    je     dividend_less_than_divisor

    mov    ebx,    divisor
    mov    eax,    divisor
    call   strlen
    add    eax,    ebx
    mov    BYTE[eax],    ASCII_ZERO
    inc    eax
    mov    BYTE[eax], 0

    call   is_enough
    cmp    BYTE[is_enough_flag],    0
    je     sub_sub

main_loop:
    call   cal_sub
    call   add_one
    call   add_one
    call   add_one
    call   add_one
    call   add_one
    call   add_one
    call   add_one
    call   add_one
    call   add_one
    call   add_one

    mov    eax,    remainder          ; eax: point to remainder
    dec    eax
    mov    ebx,    dividend           ; ebx: point to dividend
    xor    ecx,    ecx
    xor    dh,     dh                 ; record if a number other than 0 has been encountered
.number_copy_loop:
    inc    eax
    inc    ecx
    cmp    ecx,    101
    jg     .judge_is_enough
    cmp    dh,    0
    jne    .process_number_copy
    cmp    BYTE[eax],    0
    je     .number_copy_loop
.process_number_copy:
    mov    dh,     1
    mov    dl,    BYTE[eax]
    add    dl,    48
    mov    BYTE[ebx],    dl
    inc    ebx
    jmp    .number_copy_loop
  
.judge_is_enough:
    mov    BYTE[ebx],    0
    call   is_enough
    cmp    BYTE[is_enough_flag],    1
    je     main_loop


    
sub_sub:
    mov    ebx,    divisor
    mov    eax,    divisor
    call   strlen
    add    eax,    ebx
    dec    eax
    mov    BYTE[eax],    0

    mov    eax,    dividend
    call   strlen
    cmp    eax,    0
    je     print_outcome
sub_main_loop:
    call   cal_sub
    call   add_one

    mov    eax,    remainder          ; eax: point to remainder
    dec    eax
    mov    ebx,    dividend           ; ebx: point to dividend
    xor    ecx,    ecx
    xor    dh,     dh                 ; record if a number other than 0 has been encountered
.sub_number_copy_loop:
    inc    eax
    inc    ecx
    cmp    ecx,    101
    jg     .sub_judge_is_enough
    cmp    dh,    0
    jne    .sub_process_number_copy
    cmp    BYTE[eax],    0
    je     .sub_number_copy_loop
.sub_process_number_copy:
    mov    dh,     1
    mov    dl,    BYTE[eax]
    add    dl,    48
    mov    BYTE[ebx],    dl
    inc    ebx
    jmp    .sub_number_copy_loop
  
.sub_judge_is_enough:
    mov    BYTE[ebx],    0
    call   is_enough
    cmp    BYTE[is_enough_flag],    1
    je     sub_main_loop


print_outcome:
    mov    eax,    quotient
    call   number_print
    call   print_space
    mov    eax,    dividend
    call   strlen
    cmp    eax,    0
    je     .dividend_is_zero
    mov    eax,    dividend
    call   string_print
    jmp    exit
.dividend_is_zero:
    call   print_zero
    jmp    exit

   
dividend_less_than_divisor:
    call   print_zero
    call   print_space
    mov    eax,    dividend
    call   string_print


exit:
    call   print_new_line
    mov    ebx,    0
    mov    eax,    1
    int    80h


;; string_read (string -> eax; length -> ebx)
;; string_read uses eax/ebx/ecx/edx
string_read:
    pusha

    mov    ecx,    eax    ; ecx stores the string
    mov    edx,    ebx    ; edx stores the length
    mov    ebx,    0      ; stdin
    mov    eax,    3      ; sys_read
    int    80h

    popa
    ret


;; strlen (string -> eax)
;; strlen uses eax/ebx
strlen:
    push    ebx
    mov     ebx,    eax
.nextchar:
    cmp     BYTE[eax],    0    ; the last char of string is '\0'
    jz      .finished
    inc     eax
    jmp     .nextchar
.finished:
    sub     eax,    ebx    ; eax = eax - ebx (length)
    pop     ebx
    ret


;; print (string -> eax)
;; string_print uses eax/ebx/ecx/edx
string_print:
    pusha

    mov    ecx,    eax    ; ecx stores the string
    call   strlen         ; eax stores the length
    mov    edx,    eax    ; edx stores the length
    mov    ebx,    1      ; stdout
    mov    eax,    4      ; sys_write
    int    80h

    popa
    ret

print_zero:
    pusha

    mov    BYTE[special_signal],    ASCII_ZERO
    mov    ecx,    special_signal
    mov    edx,    1
    mov    ebx,    1
    mov    eax,    4
    int    80h

    popa
    ret

print_space:
    pusha

    mov    BYTE[special_signal],    SPACE
    mov    ecx,    special_signal
    mov    edx,    1
    mov    ebx,    1
    mov    eax,    4
    int    80h

    popa
    ret

print_new_line:
    pusha

    mov    BYTE[special_signal],    NEW_LINE
    mov    ecx,    special_signal
    mov    edx,    1
    mov    ebx,    1
    mov    eax,    4
    int    80h

    popa
    ret


;; input_string -> eax
;; operands -> ebx
parse_input:
.parse_loop:
    cmp    BYTE[eax],    SPACE
    je     .process_space
    cmp    BYTE[eax],    NEW_LINE
    je     .return
    mov    dl,           BYTE[eax]
    mov    BYTE[ebx],    dl
    inc    eax
    inc    ebx
    jmp    .parse_loop
.process_space:
    inc    eax
    cmp    BYTE[eax],    SPACE
    je     .process_space
.return:
    ret


divide_by_zero:
    mov    eax,    msg_divided_by_zero
    call   string_print
    call   exit


zero_devide_zero:
    mov    eax,    msg_zero_devide_zero
    call   string_print
    call   exit


is_enough:
    pusha

    mov    eax,    dividend
    call   strlen
    mov    ebx,    eax          ; ebx: length of dividend
    mov    eax,    divisor
    call   strlen               ; eax: length of divisor
    cmp    eax,    ebx
    jg     .set_not_enough_flag
    jl     .set_enough_flag

    mov    ecx,    eax          ; ecx: length
    mov    eax,    dividend     ; eax: pos of dividend
    mov    ebx,    divisor      ; ebx: pos of divisor
.is_enough_loop:
    mov    dh,     BYTE[eax]
    mov    dl,     BYTE[ebx]
    cmp    dh,     dl
    jg     .set_enough_flag
    jl     .set_not_enough_flag

    inc    eax
    inc    ebx
    dec    ecx
    cmp    ecx,    0
    je     .set_enough_flag
    jmp    .is_enough_loop
.set_not_enough_flag:
    mov    BYTE[is_enough_flag],    0
    jmp    .exit_is_enough
.set_enough_flag:
    mov    BYTE[is_enough_flag],    1
.exit_is_enough:    
    popa
    ret


add_one:
    pusha

    mov    eax,    quotient
    add    eax,    MAX_LEN_OPERAND
    dec    eax                         ; point to the last number
    xor    bh,     bh                  ; carry_flag
    inc    bh
.add_one_loop:
    mov    bl,     BYTE[eax]           ; number + 1
    add    bl,     bh
    xor    bh,     bh
    mov    BYTE[eax],    bl
    cmp    bl,     9
    jg     .process_add_one_overflow
    jmp    .exit_add_one
.process_add_one_overflow:
    sub    bl,     10
    mov    BYTE[eax],    bl
    inc    bh
    dec    eax
    jmp    .add_one_loop
.exit_add_one:
    popa
    ret


;; print (number -> eax)
number_print:
    pusha

    xor    ecx,    ecx
    xor    edx,    edx                 ; length of the number
    dec    eax
.number_print_loop:
    inc    eax
    inc    ecx
    cmp    ecx,    MAX_LEN_OPERAND
    jg     .exit_number_print
    cmp    edx,    0
    jne    .process_number
    cmp    BYTE[eax],    0
    je     .number_print_loop
.process_number:
    mov    bl,    BYTE[eax]
    add    bl,    48
    mov    BYTE[eax],    bl
    inc    edx
    jmp    .number_print_loop
.exit_number_print:
    sub    eax,    edx
    call   string_print

    popa
    ret


;; edi - esi
;; eax: the length of divisor
;; ebx: point to remainder
cal_sub:
    mov    eax,    dividend
    call   strlen
    mov    edi,    dividend
    add    edi,    eax               ; point to the last number of dividend
    mov    eax,    divisor
    call   strlen
    mov    esi,    divisor
    add    esi,    eax               ; point to the last number of divisor
    mov    ebx,    remainder
    add    ebx,    MAX_LEN_OPERAND   ; point to the last number of remainder
    xor    ch,     ch                ; ch: sub_carry
    inc    eax
.cal_sub_loop:
    dec    eax
    cmp    eax,    0
    je     .process_longer_part
    dec    edi
    dec    esi 
    dec    ebx
    mov    dh,     BYTE[edi]         ; dh = dh - dl - ch
    mov    dl,     BYTE[esi]
    add    dl,     ch
    xor    ch,     ch
    cmp    dh,     dl
    jl     .process_sub_not_enough
    sub    dh,     dl
    mov    BYTE[ebx],    dh
    jmp    .cal_sub_loop
.process_sub_not_enough:
    inc    ch
    add    dh,     10
    sub    dh,     dl
    mov    BYTE[ebx],    dh
    jmp    .cal_sub_loop
.process_longer_part:
    mov    eax,    dividend
    call   strlen
    mov    edx,    eax               ; edx: the length of dividend
    mov    eax,    divisor
    call   strlen
    sub    edx,    eax               ; edx: the length of dividend - the length of divisor
    mov    eax,    edx               ; eax: the length of dividend - the length of divisor
    cmp    eax,    0
    je     .exit_sub
    inc    eax
.process_longer_part_loop:
    dec    eax
    cmp    eax,    0
    je     .exit_sub
    dec    edi
    dec    ebx
    mov    dh,     BYTE[edi]
    mov    dl,     48
    add    dl,     ch
    xor    ch,     ch
    cmp    dh,     dl
    jl     .process_sub_not_enough_v2
    sub    dh,     dl
    mov    BYTE[ebx],    dh
    jmp    .process_longer_part_loop
.process_sub_not_enough_v2:
    inc    ch
    add    dh,     10
    sub    dh,     dl
    mov    BYTE[ebx],    dh
    jmp    .process_longer_part_loop
.exit_sub:
    ret

