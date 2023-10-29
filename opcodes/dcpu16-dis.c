#include "sysdep.h"
#include "disassemble.h"
#include "bfd.h"
#include "opcode/dcpu16.h"

static inline bool
is_vof(const dcpu16_build_argument *in)
{
  return (in->val >= VAL_AT_REG && in->val < VAL_PUSH_POP) || in->val == VAL_AT_IMM;
}

static bool
read_word(bfd_vma *ptr, struct disassemble_info *info, uint16_t *out)
{
  bfd_byte buffer[4];

  int status = (*info->read_memory_func)(*ptr, buffer, DCPU16_WORD_BYTE_SIZE, info);
  if (status != 0)
  {
    (*info->memory_error_func)(status, *ptr, info);
    return false;
  }

  *out = bfd_getl16(buffer);
  *ptr += 2;

  return true;
}

static bool
decode_opcode(uint16_t insn, dcpu16_build_instruction *out)
{
  out->op    = (insn >> 0x0) & 0b011111;
  out->b.val = (insn >> 0x5) & 0b011111;
  out->a.val = (insn >> 0xA) & 0b111111;

  for (size_t i = 0; i < dcpu16_op_map_size; i++)
  {
    const dcpu16_op_entry *e = dcpu16_op_map + i;

    if (e->bop == out->op)
    {
      if (out->op != BOP_SOP) return true;

      if (e->sop == out->b.val) return true;
    }
  }

  return false;
}

static bool
decode_argument(dcpu16_build_argument *out)
{
  out->has_imm = (out->val >= VAL_AT_IMM_REG && out->val < VAL_PUSH_POP) || out->val == VAL_AT_IMM || out->val == VAL_IMM;

  return true;
}

static bool
decode_insn(uint16_t insn, dcpu16_build_instruction *out)
{
  if (!decode_opcode(insn, out)) return false;

  if (out->op == BOP_SOP)
  {
    if (!decode_argument(&out->a)) return false;
  }
  else
  {
    if (!decode_argument(&out->b)) return false;
    if (!decode_argument(&out->a)) return false;
  }

  return true;
}

static void
print_opcode(const dcpu16_build_instruction *in, struct disassemble_info *info)
{
  if (in->op == BOP_SOP) (*info->fprintf_styled_func)(info->stream, dis_style_mnemonic, "%s", dcpu16_sop_names[in->b.val]);
  else (*info->fprintf_styled_func)(info->stream, dis_style_mnemonic, "%s", dcpu16_bop_names[in->op]);
}

static void
print_argument(const dcpu16_build_argument *in, struct disassemble_info *info, bool is_lhs)
{
  bool vof = is_vof(in);

  if (vof) (*info->fprintf_styled_func)(info->stream, dis_style_text, "[");

  bool with_reg = in->val <= VAL_EX;
  if (with_reg) 
  {
    if (in->val == VAL_PUSH_POP) (*info->fprintf_styled_func)(info->stream, dis_style_register, "%s", (is_lhs) ? "PUSH" : "POP");
    else (*info->fprintf_styled_func)(info->stream, dis_style_register, "%s", dcpu16_value_names[in->val]);
  }

  if (with_reg && in->has_imm) (*info->fprintf_styled_func)(info->stream, dis_style_text, "+");

  if (in->has_imm) (*info->fprintf_styled_func)(info->stream, dis_style_immediate, "0x%x", in->imm);
  else if (in->val >= VAL_LIT) (*info->fprintf_styled_func)(info->stream, dis_style_immediate, "0x%x", (uint16_t)(in->val - VAL_LIT - 1));

  if (vof) (*info->fprintf_styled_func)(info->stream, dis_style_text, "]");
}

static void
print_insn(const dcpu16_build_instruction *in, struct disassemble_info *info)
{
  print_opcode(in, info);

  (*info->fprintf_styled_func)(info->stream, dis_style_text, " ");

  if (in->op != BOP_SOP)
  {
    print_argument(&in->b, info, true);
    (*info->fprintf_styled_func)(info->stream, dis_style_text, ", ");
  }
  print_argument(&in->a, info, in->op == BOP_SOP);
}

int
print_insn_dcpu16(bfd_vma memaddr, struct disassemble_info *info)
{
  info->bytes_per_line = 6;
  info->bytes_per_chunk = 2;
  info->display_endian = BFD_ENDIAN_LITTLE;
  info->insn_type = dis_noninsn;

  bfd_vma ptr = memaddr;

  uint16_t insn = 0;
  if (!read_word(&ptr, info, &insn)) return -1;

  dcpu16_build_instruction p = (dcpu16_build_instruction) { 0 };

  if (!decode_insn(insn, &p)) {
    (*info->fprintf_func)(info->stream, "...");
    return -1;
  }

  if (p.b.has_imm && !read_word(&ptr, info, &p.b.imm)) return -1;
  if (p.a.has_imm && !read_word(&ptr, info, &p.a.imm)) return -1;

  info->insn_type = dis_nonbranch;
  print_insn(&p, info);

  return ptr - memaddr;
}
