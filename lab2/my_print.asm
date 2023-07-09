SECTION .text
global  string_print

string_print:
    push   edx
    push   ecx
    push   ebx
    push   eax

    push   ebp
    mov    ebp,    esp
    mov    eax,    [ebp+24]
    call   strlen             ; eax stores the length

    mov    edx,    eax        ; edx stores the length
    mov    ecx,    [ebp+24]   ; ecx stores the string
    mov    ebx,    1          ; stdout
    mov    eax,    4          ; sys_write
    int    80h
    pop    ebp

    pop    eax
    pop    ebx
    pop    ecx
    pop    edx
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
    sub     eax,    ebx        ; eax = eax - ebx (length)
    pop     ebx
    ret