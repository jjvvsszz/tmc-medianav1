/* Layout espelha os .exe que ja rodam na MN1: base 0x10000, .text em 0x11000 */
ENTRY(_start)
SECTIONS {
  . = 0x11000;
  .text : { *(.text.start) *(.text .text.*) }
  . = ALIGN(0x1000);
  .rdata : { *(.rodata .rodata.*) }
  . = ALIGN(0x1000);
  .data : { __iat_start = .; *(.iat) __iat_end = .; *(.data .data.* .sdata .sdata.*) }
  .bss (NOLOAD) : { __bss_start = .; *(.sbss .sbss.* .bss .bss.* COMMON) __bss_end = .; }
  /DISCARD/ : { *(.reginfo) *(.MIPS.abiflags) *(.MIPS.options) *(.pdr) *(.comment) *(.note*) *(.gnu*) *(.mdebug*) *(.eh_frame*) }
}
