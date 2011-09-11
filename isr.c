#include "system.h"

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();


/*
 * GDT.Code is the Selector for the Code segment.
 * As "GDT.Code" is not a valid symbol in C (b/c of the dot),
 * an alternative symbol "GDT_CODE" is defined and exported in assembler.
 * That symbol is not a Pointer, but we're only interested in its integer value.
 * As C can only import symbols pointing to something, a pseudo integer is
 * declared and a Macro "GDT_Code_Sel" extracts the value (address) and
 * casts it to unsinged short (via unsigned long to avoid warnings).
 */
extern int GDT_Code;
#define GDT_Code_Sel ((unsigned short)(((unsigned long)&GDT_Code)&0xFFFF))

void isr_install()
{
    idt_set_gate(0,  (unsigned long)isr0,  GDT_Code_Sel, 0x8E);
    idt_set_gate(1,  (unsigned long)isr1,  GDT_Code_Sel, 0x8E);
    idt_set_gate(2,  (unsigned long)isr2,  GDT_Code_Sel, 0x8E);
    idt_set_gate(3,  (unsigned long)isr3,  GDT_Code_Sel, 0x8E);
    idt_set_gate(4,  (unsigned long)isr4,  GDT_Code_Sel, 0x8E);
    idt_set_gate(5,  (unsigned long)isr5,  GDT_Code_Sel, 0x8E);
    idt_set_gate(6,  (unsigned long)isr6,  GDT_Code_Sel, 0x8E);
    idt_set_gate(7,  (unsigned long)isr7,  GDT_Code_Sel, 0x8E);
    idt_set_gate(8,  (unsigned long)isr8,  GDT_Code_Sel, 0x8E);
    idt_set_gate(9,  (unsigned long)isr9,  GDT_Code_Sel, 0x8E);
    idt_set_gate(10, (unsigned long)isr10, GDT_Code_Sel, 0x8E);
    idt_set_gate(11, (unsigned long)isr11, GDT_Code_Sel, 0x8E);
    idt_set_gate(12, (unsigned long)isr12, GDT_Code_Sel, 0x8E);
    idt_set_gate(13, (unsigned long)isr13, GDT_Code_Sel, 0x8E);
    idt_set_gate(14, (unsigned long)isr14, GDT_Code_Sel, 0x8E);
    idt_set_gate(15, (unsigned long)isr15, GDT_Code_Sel, 0x8E);
    idt_set_gate(16, (unsigned long)isr16, GDT_Code_Sel, 0x8E);
    idt_set_gate(17, (unsigned long)isr17, GDT_Code_Sel, 0x8E);
    idt_set_gate(18, (unsigned long)isr18, GDT_Code_Sel, 0x8E);
    idt_set_gate(19, (unsigned long)isr19, GDT_Code_Sel, 0x8E);
    idt_set_gate(20, (unsigned long)isr20, GDT_Code_Sel, 0x8E);
    idt_set_gate(21, (unsigned long)isr21, GDT_Code_Sel, 0x8E);
    idt_set_gate(22, (unsigned long)isr22, GDT_Code_Sel, 0x8E);
    idt_set_gate(23, (unsigned long)isr23, GDT_Code_Sel, 0x8E);
    idt_set_gate(24, (unsigned long)isr24, GDT_Code_Sel, 0x8E);
    idt_set_gate(25, (unsigned long)isr25, GDT_Code_Sel, 0x8E);
    idt_set_gate(26, (unsigned long)isr26, GDT_Code_Sel, 0x8E);
    idt_set_gate(27, (unsigned long)isr27, GDT_Code_Sel, 0x8E);
    idt_set_gate(28, (unsigned long)isr28, GDT_Code_Sel, 0x8E);
    idt_set_gate(29, (unsigned long)isr29, GDT_Code_Sel, 0x8E);
    idt_set_gate(30, (unsigned long)isr30, GDT_Code_Sel, 0x8E);
    idt_set_gate(31, (unsigned long)isr31, GDT_Code_Sel, 0x8E);

}

char *exception_messages[] =
{
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",

    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",

    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",

    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

void fault_handler(struct regs *r)
{
    if (r->int_no < 32) {
        printf("\nException: %s\n", exception_messages[r->int_no]);
        printf("System halted.\n");
        while(1) {};
    }
}

