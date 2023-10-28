#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"

const bfd_arch_info_type bfd_dcpu16_arch =
{
  16, // Word size in bits
  16, // Address size in bits
  8, // Byte size in bits
  bfd_arch_dcpu16, // Arch enum
  bfd_mach_dcpu16, // Machine number
  "dcpu16", // Arch name
  "dcpu16", // Printable name
  4, // TODO: Alignment
  true, // Is default?
  bfd_default_compatible,
  bfd_default_scan,
  bfd_arch_default_fill,
  NULL,
  0
};
