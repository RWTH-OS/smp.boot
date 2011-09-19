
GLOBAL Realm32
EXTERN main

[BITS 32]
Realm32:
    mov ax, 0x0F00+'2'
    mov [0xB8000], ax
    mov ax, 0x0F00+'0'
    mov [0xB8002], ax

    ; load GDT
    lgdt [GDT.Pointer]
    jmp GDT.Code:Realm64      ; set code segment and enter 64-bit long mode
    
[BITS 64]

Realm64:
    cli
    mov ax, GDT.Data
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ;;mov ss, ax                 ; WARNING: everything crashes, when SS is loaded!!!
    ;mov rsp, Realm32            ; all below is no more needed...
                                ; (except multiboot_info_t, but the stack will not grow that fast...)
    ; Realm32 is 0x140000 (0x40000 above 1MB).
    ; Attention: below 1MB is I/O space (no RAM)


    mov ax, 0x0F00+'3'
    mov [0xB8000], ax
    mov ax, 0x0F00+'0'
    mov [0xB8002], ax

    ;jmp endless

; switch to 64 bit; preserve multiboot_info!

    call main    ; main()

endless:
    hlt
    jmp endless

; Shortly we will add code for loading the GDT right here!

GDT:
    .Null: equ $ - GDT        ; the null descriptor
    dw 0                        ; Limit (low)
    dw 0                        ; Base (low)
    db 0                        ; Base (middle)
    db 0                        ; Access
    db 0                        ; Granularity
    db 0                        ; Base (high)
    .Code : equ $ - GDT
    dw 0                        ; Limit (low)
    dw 0                        ; Base (low)
    db 0                         ; Base (middle)
    db 10011000b                 ; Access.       p=1  dpl=00  11  c=0  r=0  a=0  (code segment)
    db 00100000b                 ; Granularity.  g=0  d=0  l=1  avl=0  seg.limit=0000    (l=1 -> 64 bit mode)
    db 0                         ; Base (high).
    .Data: equ $ - GDT         ; The data descriptor.
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 10010010b                 ; Access.       p=1  dpl=00  10  e=0  w=1  a=0  (data segment)
                                 ; 14.9.2011, Robert Uhl: w=1 to avoid #GP after interrupt handler
    db 00000000b                 ; Granularity.  g=0  d/b=0  0  avl=0  seg.limit=0000
    db 0                         ; Base (high).
    .Pointer:                    ; The GDT-pointer.
    dw $ - GDT - 1             ; Limit.
    dq GDT                     ; Base.

global GDT_Code
GDT_Code : equ GDT.Code

; In just a few pages in this tutorial, we will add our Interrupt
; Service Routines (ISRs) right here!

global idt_load
extern idtp
idt_load:
    lidt [idtp]
    ret

; define two multiline macros for interrupt stubs
; one with pushing a dummy error code,
; the other without (because these exceptions push an error code themselves)
%macro ISR_EXCEPTION_WITHOUT_ERRCODE 1
    global isr %+ %1
    isr %+ %1 :
        push rax
        mov ax, 0x0F00+'X'
        mov [0xB8000], ax
        mov ax, 0x0F00+'0' ;('0'+%1/10)
        mov [0xB8002], ax
        ;mov ax, 0x0F00+('0'+%1 % 10)
        ;mov [0xB8004], ax
        pop rax

    .endless:
        ;hlt
        ;jmp .endless

        push QWORD 0
        push QWORD %1
        jmp isr_common_stub
%endmacro

%macro ISR_EXCEPTION_WITH_ERRCODE 1
    global isr %+ %1
    isr %+ %1 :
        ; Errcode is already on the stack
        push QWORD %1
        jmp isr_common_stub
%endmacro

ISR_EXCEPTION_WITHOUT_ERRCODE 0
ISR_EXCEPTION_WITHOUT_ERRCODE 1
ISR_EXCEPTION_WITHOUT_ERRCODE 2
ISR_EXCEPTION_WITHOUT_ERRCODE 3
ISR_EXCEPTION_WITHOUT_ERRCODE 4
ISR_EXCEPTION_WITHOUT_ERRCODE 5
ISR_EXCEPTION_WITHOUT_ERRCODE 6
ISR_EXCEPTION_WITHOUT_ERRCODE 7
ISR_EXCEPTION_WITH_ERRCODE 8
ISR_EXCEPTION_WITHOUT_ERRCODE 9
ISR_EXCEPTION_WITH_ERRCODE 10
ISR_EXCEPTION_WITH_ERRCODE 11
ISR_EXCEPTION_WITH_ERRCODE 12
ISR_EXCEPTION_WITH_ERRCODE 13
ISR_EXCEPTION_WITH_ERRCODE 14
ISR_EXCEPTION_WITHOUT_ERRCODE 15
ISR_EXCEPTION_WITHOUT_ERRCODE 16
ISR_EXCEPTION_WITHOUT_ERRCODE 17
ISR_EXCEPTION_WITHOUT_ERRCODE 18
ISR_EXCEPTION_WITHOUT_ERRCODE 19
ISR_EXCEPTION_WITHOUT_ERRCODE 20
ISR_EXCEPTION_WITHOUT_ERRCODE 21
ISR_EXCEPTION_WITHOUT_ERRCODE 22
ISR_EXCEPTION_WITHOUT_ERRCODE 23
ISR_EXCEPTION_WITHOUT_ERRCODE 24
ISR_EXCEPTION_WITHOUT_ERRCODE 25
ISR_EXCEPTION_WITHOUT_ERRCODE 26
ISR_EXCEPTION_WITHOUT_ERRCODE 27
ISR_EXCEPTION_WITHOUT_ERRCODE 28
ISR_EXCEPTION_WITHOUT_ERRCODE 29
ISR_EXCEPTION_WITHOUT_ERRCODE 30
ISR_EXCEPTION_WITHOUT_ERRCODE 31


extern fault_handler
isr_common_stub:
    ; save complete context (multi-purpose&integer, no fpu/sse)
    ; the structure is also defined in system.h:struct regs
     push rax
     push rcx
     push rdx
     push rbx
     push Qword 0       ; FIXME: why is this 0 pushed?!
     push rbp
     push rsi
     push rdi
     push r8
     push r9
     push r10
     push r11
     push r12
     push r13
     push r14
     push r15
  
     xor rax, rax
     mov  ax, ds        ; Data Segment Descriptor
     push rax

     mov ax, es         ; ES
     push rax

     push fs            ; FS + GS
     push gs

     mov rax, cr3       ; CR 3
     push rax

     mov rdi, rsp           ; Param in RDI (RSP == pointer to saved registers on stack)
     call fault_handler     ; fault_handler(struct regs *r)

     ; restore context from stack
     pop rax
     mov cr3, rax

     pop gs
     pop fs
     
     pop rax
     mov es, ax

     pop rax
     mov ds, ax

     pop r15
     pop r14
     pop r13
     pop r12
     pop r11
     pop r10
     pop r9
     pop r8
     pop rdi
     pop rsi
     pop rbp
     add rsp, 1*8           ; pushed 0 (why?)
     pop rbx
     pop rdx
     pop rcx
     pop rax

     add rsp, 2*8           ; two elements: err_code and vector-number

     iretq

[BITS 16]
global smp_start
global smp_var
global smp_end
smp_start:
    ; set Segments
    MOV ax, cs
    MOV ds, ax                              ; initialize data segment equal to code segment
    MOV ax, 0xB800
    MOV es, ax
    ; print a "I'm here"-Message
    MOV di, WORD [ds:smp_var-smp_start]     ; access smp_var relative to smp_start!
    MOV BYTE [es:di], '.'

    ; TODO : switch to 32 bit mode


    ; TODO : switch to 64 bit mode
    mov eax, 10100000b
    mov cr4, eax

    mov edx, 0x1000         ; know-how (set up in start64.asm, but we can't access symbols from there)
                            ; TODO: define global option (config.h?) where to put pml4
    mov cr3, edx

    mov ecx, 0xC000_0080
    rdmsr
    or eax, 0x0100
    wrmsr

    mov ebx, cr0
    or ebx, 0x8000_0001
    mov cr0, ebx

    MOV BYTE [es:di], 'o'
[BITS 32]
    lgdt [GDT.Pointer]


    jmp GDT.Code:Smp64      ; set code segment and enter 64-bit long mode


.halt:
    hlt
    jmp .halt
smp_var dw 0x000c                           ; local variable for the position of output
smp_end:
    nop
   

[BITS 64] 
Smp64:
    ; TODO : set up a stack pointer rsp

    MOV BYTE [0xb800E], 'O'


    ; TODO : call smp_main()
.halt:
    hlt
    jmp .halt
