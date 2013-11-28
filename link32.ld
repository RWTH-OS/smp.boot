OUTPUT_FORMAT("binary")
ENTRY(start)
phys = 0x00100000;
SECTIONS
{
  .text phys : AT(phys) {
    code = .;
    *(.text*)
    *(.rodata*)
    . = ALIGN(4096);
  }
  /*.data : AT(phys + (data - code))*/
  .data : AT( ADDR(.text) + SIZEOF(.text) )
  {
    data = .;
    *(.data)
    . = ALIGN(4096);
  }
  /*.bss : AT(phys + (bss - code)) */
  .bss : AT( ADDR(.data) + SIZEOF(.data) )
  {
    bss = .;
    *(.bss)
    . = ALIGN(4096);
  }
  end = .;
}
