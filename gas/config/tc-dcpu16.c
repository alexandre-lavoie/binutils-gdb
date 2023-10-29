#include "as.h"
#include "bfd.h"
#include "opcode/dcpu16.h"

const char *md_shortopts = "";
struct option md_longopts[] = {};
size_t md_longopts_size = sizeof(md_longopts);

const char comment_chars[] = "";
const char line_comment_chars[] = ";";
const char line_separator_chars[] = "";

const char EXP_CHARS[] = "";
const char FLT_CHARS[] = "";

const pseudo_typeS md_pseudo_table[] = {{ (char*)0, (void(*)(int))0, 0 }};

void
md_show_usage(FILE *stream ATTRIBUTE_UNUSED)
{
  fprintf(stream, "\nDCPU16 options: None\n");
}

int
md_parse_option(int c ATTRIBUTE_UNUSED, const char *arg ATTRIBUTE_UNUSED)
{
  return 0;
}

static htab_t dcpu16_op_hsh;
static htab_t dcpu16_value_hsh;

void
md_begin(void)
{
  dcpu16_op_hsh = str_htab_create();
  know(dcpu16_op_hsh != NULL);

  for (size_t i = 0; i < dcpu16_op_map_size; i++)
  {
    const dcpu16_op_entry *entry = dcpu16_op_map + i;
    const char *name = entry->name;

    str_hash_insert(dcpu16_op_hsh, name, (void*)entry, 0);
  }

  dcpu16_value_hsh = str_htab_create();
  know(dcpu16_value_hsh != NULL);

  for (size_t i = 0; i < dcpu16_value_map_size; i++)
  {
    const dcpu16_value_entry *entry = dcpu16_value_map + i;
    const char *name = entry->name;

    symbolS *sym = symbol_new(name, reg_section, &zero_address_frag, i);
    know(sym != NULL);

    str_hash_insert(dcpu16_value_hsh, name, (void*)sym, 0);
  }
}

static bool
parse_opcode(char **str, const dcpu16_op_entry **op)
{
  char *ptr = *str;

  *op = str_hash_find_n(dcpu16_op_hsh, ptr, 3);
  if (*op == NULL) 
  {
    as_bad(_("Opcode not found"));
    return false;
  }

  *str = ptr + 4;
  return true;
}

static bool
parse_register(char **str, expressionS *exp) 
{
  char *ptr = *str;
  while (*ptr >= 'A' && *ptr <= 'Z') ptr++;
  if (ptr <= *str) return false;

  symbolS *sym = str_hash_find_n(dcpu16_value_hsh, *str, ptr - *str);
  if (sym == NULL) return false;

  exp->X_op = O_register;
  exp->X_add_number = S_GET_VALUE(sym);

  *str = ptr;
  return true;
}

static bool
parse_immediate(char **str, expressionS *exp)
{
  char *ptr = *str;

  char radix;
  if (strncmp(ptr, "0x", 2) == 0) radix = 16;
  else if (strncmp(ptr, "0b", 2) == 0) radix = 2;
  else radix = 10;

  char *end;
  uint16_t result = strtol(ptr, &end, radix);

  if (ptr == end) return false;

  exp->X_op = O_big;
  exp->X_add_number = result;

  *str = end;
  return true;
}

#define is_valid_symbol_char(v) (v >= 'A' && v <= 'Z') || (v >= 'a' && v <= 'z') || (v >= '0' && v <= '9') || v == '_'

static bool
parse_symbol(char **str, expressionS *exp)
{
  char *ptr = *str;

  if (*ptr >= '0' && *ptr <= '9') return false;
  while (is_valid_symbol_char(*ptr)) ptr++;

  if (ptr == *str) return false;
  size_t size = ptr - *str;

  char buffer[DCPU16_SYMBOL_BYTE_SIZE] = { 0 };
  strncpy(buffer, *str, (size < DCPU16_SYMBOL_BYTE_SIZE) ? size : DCPU16_SYMBOL_BYTE_SIZE);

  symbolS *sym = symbol_find_or_make(buffer);
  know(sym != NULL);

  exp->X_op = O_symbol;
  exp->X_add_symbol = sym;

  *str = ptr;
  return true;
}

static bool
parse_argument_types(char **str, dcpu16_parse_argument *out)
{
  char *ptr = *str;

  if (parse_register(&ptr, &out->reg));
  else if (parse_immediate(&ptr, &out->imm));
  else if (parse_symbol(&ptr, &out->imm));
  else 
  {
    as_bad(_("Invalid argument type"));
    return false;
  }

  *str = ptr;
  return true;
}

static bool
parse_argument(char **str, dcpu16_parse_argument *out) 
{
  char *ptr = *str;

  bool is_value_of = *ptr == '[';
  if (is_value_of) 
  {
    ptr++;

    out->reg.X_md |= DCPU16_VALUE_OF_MASK;
    out->imm.X_md |= DCPU16_VALUE_OF_MASK;
  }

  if (!parse_argument_types(&ptr, out)) return false;

  if (*ptr == '+') 
  {
    ptr++;

    bool with_reg = out->reg.X_op != 0;

    if (!parse_argument_types(&ptr, out)) return false;

    if (
      (with_reg && out->imm.X_op == 0)
      || (!with_reg && out->reg.X_op == 0)
    )
    {
      as_bad(_("Invalid add argument type"));
      return false;
    }
  }

  if (is_value_of && *(ptr++) != ']')
  {
    as_bad(_("Missing closing bracket"));
    return false;
  }

  *str = ptr;
  return true;
}

static bool
parse_instruction(char **str, dcpu16_parse_instruction *out)
{
  char *ptr = *str;

  if (!parse_opcode(&ptr, &out->op)) return false;

  if (!parse_argument(&ptr, &out->lhs)) return false;

  bool is_double_arg = out->op->bop != BOP_SOP;
  if (is_double_arg) {
    if (*(ptr++) != ',') 
    {
      as_bad(_("Expected comma"));
      return false;
    }

    if (!parse_argument(&ptr, &out->rhs)) return false;
  }

  if (*ptr != ';' && *ptr != '\0')
  {
    as_bad(_("Expected end of line"));
    return false;
  }

  *str = ptr;
  return true;
}

static inline void
optimize_small_immediate(const dcpu16_parse_argument *in, dcpu16_build_argument *out)
{
  if (in->imm.X_op == O_symbol) return;

  if (out->imm == UINT16_MAX) {
    out->val = VAL_LIT;
    out->has_imm = false;
  } else if (out->imm <= 0x1E) {
    out->val = VAL_LIT + (uint8_t)out->imm + 1;
    out->has_imm = false;
  }
}

static bool
is_valid_immediate(const dcpu16_parse_argument *in, dcpu16_build_argument *out)
{
  if (in->reg.X_md != 0 && in->imm.X_md != 0 && out->val > VAL_REG_J)
  {
    as_bad(_("Immediate only allowed for base registers"));
    return false;
  }

  return true;
}

static void
resolve_immediate(const dcpu16_parse_argument *in, dcpu16_build_argument *out)
{
  out->has_imm = in->imm.X_op != 0;
  if (in->imm.X_op == O_symbol) out->imm = symbol_get_value_expression(in->imm.X_add_symbol)->X_add_number;
  else if (in->imm.X_op == O_big) out->imm = in->imm.X_add_number;
}

static void
handle_immediate(const dcpu16_parse_argument *in, dcpu16_build_argument *out)
{
  bool vof = (in->reg.X_md & DCPU16_VALUE_OF_MASK) > 0;
  
  bool with_reg = in->reg.X_op != 0;
  bool with_imm = out->has_imm;

  if (with_reg && vof && out->val <= VAL_REG_J) out->val += with_imm ? VAL_AT_IMM_REG : VAL_AT_REG;

  if (!with_reg && with_imm) out->val = vof ? VAL_AT_IMM : VAL_IMM;
}

static bool
build_argument(const dcpu16_parse_argument *in, dcpu16_build_argument *out)
{
  if (in->reg.X_op != 0) out->val = dcpu16_value_map[in->reg.X_add_number].value;

  resolve_immediate(in, out);
  if (!is_valid_immediate(in, out)) return false;
  handle_immediate(in, out);

  return true;
}

static bool
build_instruction(const dcpu16_parse_instruction *in, dcpu16_build_instruction *out)
{
  const dcpu16_parse_argument *a_in;
  if (in->op->bop == BOP_SOP)
  {
    if (!build_argument(&in->lhs, &out->a)) return false;
    out->b.val = in->op->sop;
    a_in = &in->lhs;
  }
  else
  {
    if (!build_argument(&in->lhs, &out->b)) return false;
    if (!build_argument(&in->rhs, &out->a)) return false;
    a_in = &in->rhs;
  }

  optimize_small_immediate(a_in, &out->a);

  out->op = in->op->bop;

  return true;
}

static inline void
emit_opcode(const dcpu16_build_instruction *in)
{
  uint16_t insn = 
    ((in->a.val & 0b111111) << 0xA) | 
    ((in->b.val & 0b011111) << 0x5) |
    ((in->op    & 0b011111) << 0x0) ;

  char *frag = frag_more(DCPU16_INSTRUCTION_BYTE_SIZE);
  md_number_to_chars(frag, insn, DCPU16_INSTRUCTION_BYTE_SIZE);
}

static inline void
emit_immediate(const dcpu16_build_argument *in)
{
  if (!in->has_imm) return;

  char *frag = frag_more(DCPU16_IMMEDIATE_BYTE_SIZE);
  md_number_to_chars(frag, in->imm, DCPU16_INSTRUCTION_BYTE_SIZE);
}

static void
emit_instruction(const dcpu16_build_instruction *in)
{
  emit_opcode(in);

  emit_immediate(&in->b);
  emit_immediate(&in->a);
}

void
md_assemble(char *str)
{
  dcpu16_parse_instruction p = (dcpu16_parse_instruction) { 0 };
  if (!parse_instruction(&str, &p)) return;

  dcpu16_build_instruction b = (dcpu16_build_instruction) { 0 };
  if (!build_instruction(&p, &b)) return;

  emit_instruction(&b);
}

symbolS
*md_undefined_symbol(char *name ATTRIBUTE_UNUSED)
{
  return 0;
}

const char
*md_atof(int type ATTRIBUTE_UNUSED, char *lit ATTRIBUTE_UNUSED, int *size ATTRIBUTE_UNUSED)
{
  return 0;
}

valueT
md_section_align(asection *seg ATTRIBUTE_UNUSED, valueT size ATTRIBUTE_UNUSED)
{
  int align = bfd_section_alignment(seg);
  int new_size = ((size + (1 << align) -1) & -(1 << align));

  return new_size;
}

void
md_convert_frag(bfd *abfd ATTRIBUTE_UNUSED, asection *seg ATTRIBUTE_UNUSED, fragS *fragp ATTRIBUTE_UNUSED)
{
  return;
}

void
md_apply_fix(fixS *fixp ATTRIBUTE_UNUSED, valueT *val ATTRIBUTE_UNUSED, segT seg ATTRIBUTE_UNUSED)
{
  return;
}

arelent
*tc_gen_reloc(asection *seg ATTRIBUTE_UNUSED, fixS *fixp ATTRIBUTE_UNUSED)
{
  return 0;
}

long
md_pcrel_from(fixS *fixp ATTRIBUTE_UNUSED)
{
  as_fatal(_("unexpected call"));
  return 0;
}

int
md_estimate_size_before_relax(fragS *fragp ATTRIBUTE_UNUSED, asection *seg ATTRIBUTE_UNUSED)
{
  as_fatal(_("unexpected call"));
  return 0;
}
