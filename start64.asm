; This is the kernel's entry point. We could either call main here,
; or we can use this to setup the stack or other nice stuff, like
; perhaps setting up the GDT and segments. Please note that interrupts
; are disabled at this point: More on interrupts later!
[BITS 32]
global start
start:
    mov esp, _sys_stack     ; This points the stack to our new stack area
    jmp stublet

; This part MUST be 4byte aligned, so we solve that issue using 'ALIGN 4'
ALIGN 4
mboot:
    ; Multiboot macros to make a few lines later more readable
    MULTIBOOT_PAGE_ALIGN	equ 1<<0
    MULTIBOOT_MEMORY_INFO	equ 1<<1
    MULTIBOOT_AOUT_KLUDGE	equ 1<<16
    MULTIBOOT_HEADER_MAGIC	equ 0x1BADB002
    MULTIBOOT_HEADER_FLAGS	equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_AOUT_KLUDGE
    MULTIBOOT_CHECKSUM	equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
    EXTERN code, bss, end

    ; This is the GRUB Multiboot header. A boot signature
    dd MULTIBOOT_HEADER_MAGIC
    dd MULTIBOOT_HEADER_FLAGS
    dd MULTIBOOT_CHECKSUM
    
    ; AOUT kludge - must be physical addresses. Make a note of these:
    ; The linker script fills in the data for these ones!
    dd mboot
    dd code
    dd bss
    dd end
    dd start

mbi dd 0

stublet:
    mov [mbi], ebx
    
    ; debug output
    mov ax, 0x0F00+'0'
    mov [0xB8000], ax
    mov ax, 0x0F00+'0'
    mov [0xB8002], ax

    ; clear screen
    mov ecx, 80*25
    mov ax, 0x0F00+' '
.loop:
    dec ecx
    mov [0xB8000+ecx*2], ax
    jnz .loop


    ; the following bootstrap code is taken from
    ; http://wiki.osdev.org/User:Stephanvanschaik/Setting_Up_Long_Mode

    ; detect, if CPUID is available.
    ; set ID bit EFLAGS[21]
    pushfd              ; store FLAGS register
    pop eax             ; get FLAGS into EAX
    mov ecx, eax        ; ECX := EAX = FLAGS
    xor eax, 1 << 21    ; flip the ID bit (21)
    push eax
    popfd               ; FLAGS := EAX
    ; check if ID bit was successfully changed
    pushfd
    pop eax
    push ecx
    popfd               ; restore FLAGS (to ECX, previously saved)
    xor eax, ecx
    jz NoCPUID
    ; CPUID is available to use


    mov ax, 0x0F00+'0'
    mov [0xB8000], ax
    mov ax, 0x0F00+'1'
    mov [0xB8002], ax
    

    ; check if ext'd CPUID function 8000_0001 is available
    mov eax, 0x8000_0000
    cpuid
    cmp eax, 0x8000_0001
    jb NoLongMode

    ; check if long mode is available: CPUID[8000_0001]:EDX[29]
    mov eax, 0x8000_0001
    cpuid
    test edx, 1 << 29
    jz NoLongMode

    mov ax, 0x0F00+'0'
    mov [0xB8000], ax
    mov ax, 0x0F00+'2'
    mov [0xB8002], ax
    

    ; disable 32-bit paging (just in case it was activated...)
    mov eax, cr0
    and eax, ~(1<<31)
    mov cr0, eax

    mov ax, 0x0F00+'0'
    mov [0xB8000], ax
    mov ax, 0x0F00+'3'
    mov [0xB8002], ax


    ; prepare page tables
    ; first, set entries to 0
    mov edi, 0x1000             ; page tables are put here. (TODO: Why?!)
    mov cr3, edi                ; set cr3 to the PML4T
    xor eax, eax
    mov ecx, 4096               ; 4k 
    rep stosd                   ; 4k * 4 = 16kB

    ; initialize identity paging (Flags: R/W (read and write), P (present))
    mov edi, cr3                ; edi = PML4T (0x1000)
    mov DWORD [edi], 0x2003     ; PML4T[0]=[0x1000] := 0x2003 (Frame 2; R/W, P)
    add edi, 0x1000             ; edi = PDPT (0x2000, Frame 2)
    mov DWORD [edi], 0x3003     ; PDPT[0]=[0x2000] := 0x3003 (Frame 3; R/W, P)
    add edi, 0x1000             ; edi = PDT (0x3000, Frame 3)
    mov DWORD [edi], 0x4003     ; PDT[0]=[0x3000] := 0x4003 (Frame 4, R/W, P)

    add edi, 0x1000             ; edi = PTE (0x4000, Frame 4)
    mov ebx, 0x0000_0003        ; first frame (0)
    mov ecx, 512
.SetEntry:
    mov DWORD [edi], ebx
    add ebx, 0x1000
    add edi, 8
    loop .SetEntry              ; counting ECX down (from 512)

    ; enable PAE (cr4[5])
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax


    mov ax, 0x0F00+'0'
    mov [0xB8000], ax
    mov ax, 0x0F00+'4'
    mov [0xB8002], ax

    ; the switch from real-mode

    ; set LM-bit
    mov ecx, 0xC000_0080        ; EFER MSR
    rdmsr
    or eax, 1<<8                ; LM-bit (EFER MSR[8])
    wrmsr
    ; enable paging and protected mode
    mov eax, cr0
    or eax, 1<<31 | 1 <<0       ; PG ( cr0[31] ) and PM ( cr0[0] )
    mov cr0, eax

    ; now in compatibility mode

    mov ax, 0x0F00+'0'
    mov [0xB8000], ax
    mov ax, 0x0F00+'4'
    mov [0xB8002], ax


    ; load GDT
    lgdt [GDT64.Pointer]
    jmp GDT64.Code:Realm64      ; set code segment and enter 64-bit long mode

[BITS 64]
Realm64:
    cli
    mov ax, GDT64.Data
    mov ds, ax
    mov ex, ax
    mov fs, ax
    mov gs, ax


    ;jmp endless

    mov ax, 0x0F00+'0'
    mov [0xB8000], ax
    mov ax, 0x0F00+'4'
    mov [0xB8002], ax

    hlt

    mov ebx, [mbi]

; TODO: switch to 64 bit; preserve multiboot_info!
extern main

    push ebx     ; ebx should be the pointer to the multiboot_info structure
    call main

endless:
    hlt
    jmp endless


NoCPUID:
    mov eax, 'E'
    mov [0xB8000], eax
    mov eax, '1'
    mov [0xB8002], eax
    jmp endless

NoLongMode:
    mov eax, 'E'
    mov [0xB8000], eax
    mov eax, '2'
    mov [0xB8002], eax
    jmp endless


; Shortly we will add code for loading the GDT right here!

GDT64:
    .Null: epu $ - GDT64        ; the null descriptor
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



; Here is the definition of our BSS section. Right now, we'll use
; it just to store the stack. Remember that a stack actually grows
; downwards, so we declare the size of the data before declaring
; the identifier '_sys_stack'
SECTION .bss
    resb 8192               ; This reserves 8KBytes of memory here
_sys_stack:
