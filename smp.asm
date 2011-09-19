%include "config.inc"

global smp_start, smp_apid, smp_end
%define SMP_OFFSET   (SMP_FRAME<<12)

[BITS 16]
smp_start:
    cli
    ; set Segments
    MOV ax, cs
    MOV ds, ax                              ; initialize data segment equal to code segment
    MOV ax, 0xB800
    MOV es, ax                              ; initialize extra segment to video ram
    ; print a "I'm here"-Message
    XOR edi, edi
    MOV di, WORD [ds:smp_apid-smp_start]    ; access smp_apid relative to smp_start!

    MOV BYTE [es:12+2*edi], '.'

    ; switch to 32 bit mode

    ;MOV BYTE [es:12+2*edi], '0'

    LGDT [ds:GDT_SMP.Pointer-smp_start]     ; load GDT (relative to smp_start)
    ;MOV BYTE [es:12+2*edi], '1'

    MOV EAX, CR0
    OR AL, 1                                ; set P-Mode flag in cr0
    MOV CR0, EAX

    ;MOV BYTE [es:12+2*edi], '2'

    JMP dword GDT_SMP.Code:(SMP_OFFSET+Smp32-smp_start)  
                                            ; far jump into Code Segment, offset now 0

[BITS 32]
Smp32:
    mov ax, GDT_SMP.Data
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ; TODO: stack pointer!

    XOR edi, edi
    MOV di, [SMP_OFFSET+smp_apid-smp_start]
    MOV BYTE [0xB800C+2*edi], '3'

    ; set up stack (use frames above SMP_OFFSET
    mov eax, edi            ; 1, 2, 3, ... 16(MAX_CPU)
    inc eax                 ; 2, 3, 4, ... 17
    shl eax, 12             ; 0x2000, 0x3000, 0x4000, ..., 0x11000
    add eax, SMP_OFFSET     ; 0x8A, 0x8B, 0x8C, 0x8D, ..., 0x99
    mov esp, eax

    ; now jump into upper memory
    jmp dword GDT_SMP.Code:apStartup32

smp_apid dw 0x0001              ; local variable for the Application Processor's ID

align 64
GDT_SMP:
    .Null: equ $ - GDT_SMP      ; the null descriptor
    dw 0                        ; Limit (low)
    dw 0                        ; Base (low)
    db 0                        ; Base (middle)
    db 0                        ; Access
    db 0                        ; Granularity
    db 0                        ; Base (high)
    .Code : equ $ - GDT_SMP      ; The code descriptor.
    dw 0xFFFF                    ; Limit (low)
    dw 0                         ; Base (low)
    db 0                         ; Base (middle)
    db 0x9A                      ; Access.       p=1  dpl=00  11  c=0  r=0  a=0  (code segment)
    db 0xCF                      ; Granularity. 
    db 0                         ; Base (high).
    .Data: equ $ - GDT_SMP       ; The data descriptor.
    dw 0xFFFF                    ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 0x92                      ; Access.       p=1  dpl=00  10  e=0  w=0  a=0  (data segment)
    db 0xCF                      ; Granularity.
    db 0                         ; Base (high).
    .Pointer:                    ; The GDT-pointer.
    dw $ - GDT_SMP - 1                  ; Limit.
    dd SMP_OFFSET + (GDT_SMP-smp_start) ; Base. (linear address)


smp_end:
    nop

apStartup32:
