#include "sysdep.h"
#include "disassemble.h"
#include "bfd.h"
#include "opcode/dcpu16.h"

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
decode_insn(uint16_t insn, dcpu16_instruction *out)
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

static void
print_opcode(const dcpu16_instruction *in, struct disassemble_info *info)
{
  if (in->op == BOP_SOP) (*info->fprintf_styled_func)(info->stream, dis_style_mnemonic, "%s", dcpu16_sop_names[in->b.val]);
  else (*info->fprintf_styled_func)(info->stream, dis_style_mnemonic, "%s", dcpu16_bop_names[in->op]);
}

static void
print_register(const dcpu16_argument *in, struct disassemble_info *info, bool is_lhs)
{
  if (in->val > VAL_EX) return;

  if (in->val == VAL_PUSH_POP) (*info->fprintf_styled_func)(info->stream, dis_style_register, "%s", (is_lhs) ? "PUSH" : "POP");
  else (*info->fprintf_styled_func)(info->stream, dis_style_register, "%s", dcpu16_value_names[in->val]);
}

static void
print_immediate(const dcpu16_argument *in, struct disassemble_info *info)
{
  if (val_has_imm(in->val)) (*info->fprintf_styled_func)(info->stream, dis_style_immediate, "0x%x", in->imm);
  else if (in->val >= VAL_LIT) (*info->fprintf_styled_func)(info->stream, dis_style_immediate, "0x%x", (uint16_t)(in->val - VAL_LIT - 1));
}

static void
print_argument(const dcpu16_argument *in, struct disassemble_info *info, bool is_lhs)
{
  bool vof = val_is_vof(in->val);

  if (vof) (*info->fprintf_styled_func)(info->stream, dis_style_text, "[");

  print_register(in, info, is_lhs);

  if (val_has_reg(in->val) && val_has_imm(in->val)) (*info->fprintf_styled_func)(info->stream, dis_style_text, "+");

  print_immediate(in, info);

  if (vof) (*info->fprintf_styled_func)(info->stream, dis_style_text, "]");
}

static void
print_insn(const dcpu16_instruction *in, struct disassemble_info *info)
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

  uint16_t insn_asm = 0;
  if (!read_word(&ptr, info, &insn_asm)) return -1;

  dcpu16_instruction insn = (dcpu16_instruction) { 0 };

  if (!decode_insn(insn_asm, &insn)) {
    (*info->fprintf_func)(info->stream, "...");
    return -1;
  }

  if (insn.op != BOP_SOP && val_has_imm(insn.b.val) && !read_word(&ptr, info, &insn.b.imm)) return -1;
  if (val_has_imm(insn.a.val) && !read_word(&ptr, info, &insn.a.imm)) return -1;

  info->insn_type = dis_nonbranch;
  print_insn(&insn, info);

  return ptr - memaddr;
}
