#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"

#include "elf-bfd.h"
#include "elf/dcpu16.h"

struct dcpu16_relocation_map
{
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned char elf_reloc_val;
};

static const struct dcpu16_relocation_map dcpu16_reloc_map[] =
{
  { BFD_RELOC_NONE, R_DCPU16_NONE },
  { BFD_RELOC_16,   R_DCPU16_16 }
};

static reloc_howto_type dcpu16_elf_howto_table[] =
{
  HOWTO(R_DCPU16_NONE,
    0,
    0,
    0,
    false,
    0,
    complain_overflow_dont,
    bfd_elf_generic_reloc,
    "R_DCPU16_NONE",
    false,
    0,
    0,
    false
  ),
  HOWTO(R_DCPU16_16,
    0,
    2,
    16,
    false,
    0,
    complain_overflow_bitfield,
    bfd_elf_generic_reloc,
    "R_DCPU16_16",
    false,
    0,
    0xFFFF,
    false
  )
};

static reloc_howto_type*
dcpu16_reloc_type_lookup(bfd *abfd ATTRIBUTE_UNUSED, bfd_reloc_code_real_type code)
{
  for (size_t i = 0; i < R_DCPU16_max; i++)
  {
    if (code != dcpu16_reloc_map[i].bfd_reloc_val) continue;

    unsigned int howto_index = dcpu16_reloc_map[i].elf_reloc_val;

    return &dcpu16_elf_howto_table[howto_index];
  }

  return 0;
}

#define bfd_elf32_bfd_reloc_type_lookup dcpu16_reloc_type_lookup

static reloc_howto_type*
dcpu16_reloc_name_lookup(bfd *abfd ATTRIBUTE_UNUSED, const char *name)
{
  for (size_t i = 0; i < R_DCPU16_max; i++)
  {
    const char *entry_name = dcpu16_elf_howto_table[i].name;
    if (entry_name == 0 || strcasecmp(entry_name, name) != 0) continue;

    return &dcpu16_elf_howto_table[i];
  }

  return 0;
}

#define bfd_elf32_bfd_reloc_name_lookup dcpu16_reloc_name_lookup

static bool
dcpu16_info_to_howto_rela(bfd *abfd, arelent *cache_ptr, Elf_Internal_Rela *dst)
{
  unsigned int r_type = ELF32_R_TYPE(dst->r_info);

  if (r_type >= (unsigned int)R_DCPU16_max)
  {
    _bfd_error_handler(_("%p unsupported relocation type %d"), abfd, r_type);
    bfd_set_error(bfd_error_bad_value);

    return false;
  }

  cache_ptr->howto = &dcpu16_elf_howto_table[r_type];

  return true;
}

#define elf_info_to_howto_rel NULL
#define elf_info_to_howto dcpu16_info_to_howto_rela

#undef TARGET_LITTLE_SYM
#define TARGET_LITTLE_SYM   dcpu16_elf32_vec

#undef TARGET_LITTLE_NAME
#define TARGET_LITTLE_NAME  "elf32-dcpu16"

#define ELF_ARCH            bfd_arch_dcpu16
#define ELF_MAXPAGESIZE     0x1000
#define ELF_MACHINE_CODE    EM_DCPU16
#define ELF_TARGET_ID	      GENERIC_ELF_DATA

#define elf_backend_rela_normal 1

#include "elf32-target.h"
