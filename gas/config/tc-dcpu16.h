#ifndef TC_DCPU16_H
#define TC_DCPU16_H

#define TARGET_FORMAT "elf32-dcpu16"
#define TARGET_ARCH bfd_arch_dcpu16
#define TARGET_MACH bfd_mach_dcpu16

#define TARGET_BYTES_BIG_ENDIAN 0

#define md_operand(X)

#define md_number_to_chars number_to_chars_littleendian

#define WORKING_DOT_WORD

#endif
