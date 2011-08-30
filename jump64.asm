
GLOBAL Realm32
EXTERN main

[BITS 32]
Realm32:
    mov ax, 0x0F00+'1'
    mov [0xB8000], ax
    mov ax, 0x0F00+'0'
    mov [0xB8002], ax

    ; load GDT
    lgdt [GDT64.Pointer]
    jmp GDT64.Code:Realm64      ; set code segment and enter 64-bit long mode
    
[BITS 64]

Realm64:
    cli
    mov ax, GDT64.Data
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ; TODO: what about the stack?!
    mov ss, ax
    mov rsp, Realm32            ; all below is no more needed...
                                ; (except multiboot_info_t, but the stack will not grow that fast...)


    mov ax, 0x0F00+'1'
    mov [0xB8000], ax
    mov ax, 0x0F00+'1'
    mov [0xB8002], ax


; switch to 64 bit; preserve multiboot_info!

    call next
next:
    ; rip now on stack; leave it there as first (right-most) argument to main()
    push rbx     ; ebx should be the pointer to the multiboot_info structure
    call main    ; main(multiboot_info_t *mbi, void* eip)

endless:
    hlt
    jmp endless

; Shortly we will add code for loading the GDT right here!

GDT64:
    .Null: equ $ - GDT64        ; the null descriptor
    dw 0                        ; Limit (low)
    dw 0                        ; Base (low)
    db 0                        ; Base (middle)
    db 0                        ; Access
    db 0                        ; Granularity
    db 0                        ; Base (high)
    .Code : equ $ - GDT64
    dw 0                        ; Limit (low)
    dw 0                        ; Base (low)
    db 0                         ; Base (middle)
    db 10011000b                 ; Access.
    db 00100000b                 ; Granularity.
    db 0                         ; Base (high).
    .Data: equ $ - GDT64         ; The data descriptor.
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 10010000b                 ; Access.
    db 00000000b                 ; Granularity.
    db 0                         ; Base (high).
    .Pointer:                    ; The GDT-pointer.
    dw $ - GDT64 - 1             ; Limit.
    dq GDT64                     ; Base.


; In just a few pages in this tutorial, we will add our Interrupt
; Service Routines (ISRs) right here!


