    org    10000h
    mov    ax,    cs
    mov    ds,    ax
    mov    es,    ax
    mov    ax,    0x00
    mov    ss,    ax
    mov    sp,    0x7c00

;=======   display on screen : Hello Loader!
    mov    ax,    1301h
    mov    bx,    0047h
    mov    dx,    0911h
    mov    cx,    13
    push   ax
    mov    ax,    ds
    mov    es,    ax
    pop    ax
    mov    bp,    StartLoaderMessage
    int    10h

    jmp    $

;=======   display messages
StartLoaderMessage:    db    "Hello Loader!"