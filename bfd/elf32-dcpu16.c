#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "elf-bfd.h"

static reloc_howto_type*
dcpu16_reloc_type_lookup(bfd *abfd ATTRIBUTE_UNUSED, bfd_reloc_code_real_type code ATTRIBUTE_UNUSED)
{
    return 0;
}

#define bfd_elf32_bfd_reloc_type_lookup dcpu16_reloc_type_lookup

static reloc_howto_type*
dcpu16_reloc_name_lookup(bfd *abfd ATTRIBUTE_UNUSED, const char *name ATTRIBUTE_UNUSED)
{
    return 0;
}

#define bfd_elf32_bfd_reloc_name_lookup dcpu16_reloc_name_lookup

#define ELF_ARCH            bfd_arch_dcpu16
#define ELF_MAXPAGESIZE     0x4000
#define ELF_MACHINE_CODE    EM_DCPU16

#undef TARGET_LITTLE_SYM
#define TARGET_LITTLE_SYM   dcpu16_elf32_vec

#undef TARGET_LITTLE_NAME
#define TARGET_LITTLE_NAME  "elf32-dcpu16"

#include "elf32-target.h"
