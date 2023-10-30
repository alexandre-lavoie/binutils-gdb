#ifndef _ELF_DCPU16_H
#define _ELF_DCPU16_H

#include "elf/reloc-macros.h"

START_RELOC_NUMBERS(elf_dcpu16_reloc_type)
  RELOC_NUMBER(R_DCPU16_NONE, 0)
  RELOC_NUMBER(R_DCPU16_16, 1)
END_RELOC_NUMBERS(R_DCPU16_max)

#endif /* _ELF_DCPU16_H */
