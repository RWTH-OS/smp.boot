;/*
; * =====================================================================================
; *
; *       Filename:  start.asm
; *
; *    Description:  This file contains the early boot code after the multiboot initialization 
; *                  up to the set up of the GDT and paging. This is common to 32 and 64 bit 
; *                  versions and %include'd in both.
; *
; *        Version:  1.0
; *        Created:  24.08.2011 14:44:29
; *       Revision:  none
; *       Compiler:  nasm
; *
; *         Author:  Georg Wassen (gw) (wassen@lfbs.rwth-aachen.de), 
; *        Company:  Lehrstuhl für Betriebssysteme (Chair for Operating Systems)
; *                  RWTH Aachen University
; *
; * Copyright (c) 2011, Georg Wassen, RWTH Aachen University
; * All rights reserved.
; *
; * Redistribution and use in source and binary forms, with or without
; * modification, are permitted provided that the following conditions are met:
; *    * Redistributions of source code must retain the above copyright
; *      notice, this list of conditions and the following disclaimer.
; *    * Redistributions in binary form must reproduce the above copyright
; *      notice, this list of conditions and the following disclaimer in the
; *      documentation and/or other materials provided with the distribution.
; *    * Neither the name of the University nor the names of its contributors
; *      may be used to endorse or promote products derived from this
; *      software without specific prior written permission.
; *
; * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
; * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
; * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; * DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
; * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
; * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
; * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
; * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
; * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
; *
; * =====================================================================================
; */


    mov ax, 0x0F00+'1'
    mov [0xB8000], ax
    mov ax, 0x0F00+'0'
    mov [0xB8002], ax

    ;mov [global_mbi], ebx          ; store pointer to multiboot_info for later

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


    mov ax, 0x0F00+'1'
    mov [0xB8000], ax
    mov ax, 0x0F00+'1'
    mov [0xB8002], ax
    
    mov eax, 1
    cpuid               ; CPUID(1)
    and edx, 1<<9       ; EDX[bit9] -> has localAPIC
    jz NoLocalAPIC      ; we need a local APIC
    ; local APIC is available

    mov ax, 0x0F00+'1'
    mov [0xB8000], ax
    mov ax, 0x0F00+'2'
    mov [0xB8002], ax
    


;void search_mp()
    extern get_info
    call get_info


    mov ax, 0x0F00+'1'
    mov [0xB8000], ax
    mov ax, 0x0F00+'3'
    mov [0xB8002], ax


    jmp setup

NoCPUID:                ; Error Code E1: CPUID instruction is not supported.
    mov eax, 'E'
    mov [0xB8000], eax
    mov eax, '1'
    mov [0xB8002], eax
    jmp endless

;NoMultiprocessor:
    ;mov eax, 'E'
    ;mov [0xB8000], eax
    ;mov eax, '2'
    ;mov [0xB8002], eax
    ;jmp endless

NoLocalAPIC:            ; Error Code E2: no local APIC available
    mov eax, 'E'
    mov [0xB8000], eax
    mov eax, '3'
    mov [0xB8002], eax
    jmp endless
