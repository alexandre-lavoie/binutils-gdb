#include "as.h"
#include "bfd.h"
#include "opcode/dcpu16.h"
#include <ctype.h>

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
parse_opcode(const char *name, expressionS *expr)
{
  if (strnlen(name, 4) > 3) return false;

  dcpu16_op_entry *entry = str_hash_find_n(dcpu16_op_hsh, name, 3);
  if (entry == NULL) return false;

  expr->X_op = O_op;
  expr->X_add_number = entry->bop;
  expr->X_md = entry->sop;

  return true;
}

static bool
parse_register(const char *name, expressionS *exp) 
{
  size_t size = strnlen(name, 5);
  if (size > 4) return false;

  char lower[5] = { 0 };
  for (size_t i = 0; i < size; i++) lower[i] = tolower(name[i]);

  symbolS *sym = str_hash_find_n(dcpu16_value_hsh, lower, size);
  if (sym == NULL) return false;

  exp->X_op = O_register;
  exp->X_add_number = S_GET_VALUE(sym);

  return true;
}

static bool
parse_symbol(const char *name, expressionS *exp)
{
  symbolS *sym = symbol_find_or_make(name);
  know(sym != NULL);

  exp->X_op = O_symbol;
  exp->X_add_symbol = sym;
  exp->X_add_number = 1;

  return true;
}

int
dcpu16_parse_name(const char *name, expressionS *exp, char *next_char ATTRIBUTE_UNUSED)
{
  if (parse_opcode(name, exp)) return 1;
  else if (parse_register(name, exp)) return 1;
  else if (parse_symbol(name, exp)) return 1;

  return 0;
}

static bool
parse_argument(expressionS *out)
{
  bool is_value_of = *input_line_pointer == '[';
  if (is_value_of) input_line_pointer++;

  expression(out);

  if (is_value_of)
  {
    out->X_md |= DCPU16_VALUE_OF_MASK;

    if (*input_line_pointer == ']') input_line_pointer++;
    else as_warn(_("Missing closing bracket"));
  }

  return true;
}

static bool
parse_insn(expressionS *op, expressionS *lhs, expressionS *rhs)
{
  expression(op);

  if (op->X_op != O_op)
  {
    as_bad(_("Invalid opcode"));
    return false;
  }

  SKIP_ALL_WHITESPACE();

  if (!parse_argument(lhs)) return false;

  if (op->X_add_number == BOP_SOP) return true;

  if (*(input_line_pointer++) != ',')
  {
    as_bad(_("Missing comma"));
    return false;
  }

  if (!parse_argument(rhs)) return false;

  return true;
}

#define GET_REG(i) dcpu16_value_map[i].value

static bool
build_atomic(const expressionS *exp, dcpu16_argument *out)
{
  switch (exp->X_op)
  {
    case O_register:
      out->val = GET_REG(exp->X_add_number);
      return true;
    case O_constant:
      out->val = VAL_IMM;
      out->imm = exp->X_add_number;
      return true;
    case O_symbol:
      out->val = VAL_IMM;
      out->sym = make_expr_symbol(exp);
      return true;
    default:
      return false;
  }
}

static bool
build_argument(const expressionS *exp, dcpu16_argument *out)
{
  bool is_value_of = (exp->X_md & DCPU16_VALUE_OF_MASK);

  if (build_atomic(exp, out))
  {
    if (!is_value_of) goto end;

    if (val_has_reg(out->val)) out->val += VAL_AT_REG;
    else out->val = VAL_AT_IMM;
  }
  else if (exp->X_op == O_add)
  {
    if (!is_value_of)
    {
      as_bad(_("Offset only supported in value of"));
      return false;
    }

    expressionS *lhs = symbol_get_value_expression(exp->X_add_symbol);
    expressionS *rhs = symbol_get_value_expression(exp->X_op_symbol);

    dcpu16_argument larg = (dcpu16_argument) { 0 };
    dcpu16_argument rarg = (dcpu16_argument) { 0 };

    if (!build_atomic(lhs, &larg) || !build_atomic(rhs, &rarg))
    {
      as_bad(_("Offset too complex"));
      return false;
    }

    bool ilr = val_has_reg(larg.val);
    if (ilr != val_has_imm(rarg.val) && val_has_imm(larg.val) != val_has_reg(rarg.val))
    {
      as_bad(_("Offset only supports register and immediate"));
      return false;
    }

    dcpu16_argument reg_arg = ilr ? larg : rarg;
    dcpu16_argument imm_arg = ilr ? rarg : larg;

    out->val = reg_arg.val + VAL_AT_IMM_REG;
    out->imm = imm_arg.imm;
    out->sym = imm_arg.sym;
  }
  else
  {
    as_bad(_("Invalid argument type"));
    return false;
  }

  end:
  return true;
}

static bool
build_insn(const expressionS *op, const expressionS *lhs, const expressionS *rhs, dcpu16_instruction *out)
{
  uint8_t bop = op->X_add_number;
  uint8_t sop = op->X_md;

  out->op = bop;

  if (bop == BOP_SOP)
  {
    if (!build_argument(lhs, &out->a)) return false;
    out->b.val = sop;
  }
  else
  {
    if (!build_argument(lhs, &out->b)) return false;
    if (!build_argument(rhs, &out->a)) return false;
  }

  return true;
}

static inline bool
optimize_small_immediate(dcpu16_argument *out)
{
  if (out->val != VAL_IMM || out->sym != NULL) return false;

  if (out->imm == UINT16_MAX) out->val = VAL_LIT;
  else if (out->imm <= 0x1E) out->val = VAL_LIT + (uint8_t)out->imm + 1;
  else return false;

  return true;
}

static inline bool
optimize_zero_offset(dcpu16_argument *out)
{
  if (out->sym != NULL || out->val < VAL_AT_IMM_REG || out->val >= VAL_PUSH_POP) return false;

  if (out->imm != 0) return false;

  out->val -= (VAL_AT_IMM_REG - VAL_AT_REG);

  return true;
}

static void
optimize_insn(dcpu16_instruction *out)
{
  optimize_small_immediate(&out->a);

  optimize_zero_offset(&out->a);
  if (out->op != BOP_SOP) optimize_zero_offset(&out->b);
}

static inline void
emit_opcode(const dcpu16_instruction *in)
{
  uint16_t insn = 
    ((in->a.val & 0b111111) << 0xA) | 
    ((in->b.val & 0b011111) << 0x5) |
    ((in->op    & 0b011111) << 0x0) ;

  char *frag = frag_more(DCPU16_INSTRUCTION_BYTE_SIZE);
  md_number_to_chars(frag, insn, DCPU16_INSTRUCTION_BYTE_SIZE);
}

static void
emit_immediate(const dcpu16_argument *in)
{
  if (!val_has_imm(in->val)) return;

  char *frag = frag_more(DCPU16_IMMEDIATE_BYTE_SIZE);
  md_number_to_chars(frag, in->imm, DCPU16_INSTRUCTION_BYTE_SIZE);

  if (in->sym != NULL) {
    symbolS *sym = in->sym;

    expressionS *addr_expr = symbol_get_value_expression(sym);
    know(addr_expr->X_op == O_symbol);
    addr_expr->X_add_number = 0;

    int where = frag - frag_now->fr_literal;

    fix_new_exp(frag_now, where, 2, addr_expr, false, BFD_RELOC_16);
  }
}

static void
emit_insn(const dcpu16_instruction *in)
{
  emit_opcode(in);

  if (in->op != BOP_SOP) emit_immediate(&in->b);
  emit_immediate(&in->a);
}

void
md_assemble(char *str)
{
  input_line_pointer = str;

  expressionS op = (expressionS) { 0 };
  expressionS lhs = (expressionS) { 0 };
  expressionS rhs = (expressionS) { 0 };

  if (!parse_insn(&op, &lhs, &rhs)) return;

  dcpu16_instruction insn = (dcpu16_instruction) { 0 };
  if (!build_insn(&op, &lhs, &rhs, &insn)) return;

  optimize_insn(&insn);

  emit_insn(&insn);
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
md_section_align(asection *seg, valueT size)
{
  int align = bfd_section_alignment(seg);
  int new_size = ((size + (1 << align) -1) & -(1 << align));

  return new_size;
}

void
md_convert_frag(bfd *abfd ATTRIBUTE_UNUSED, asection *seg ATTRIBUTE_UNUSED, fragS *fragp ATTRIBUTE_UNUSED)
{
  as_fatal(_("unexpected call"));
}

void
md_apply_fix(fixS *fixp ATTRIBUTE_UNUSED, valueT *val ATTRIBUTE_UNUSED, segT seg ATTRIBUTE_UNUSED)
{}

arelent
*tc_gen_reloc(asection *seg ATTRIBUTE_UNUSED, fixS *fixp)
{
  gas_assert(fixp != 0);

  arelent *reloc = XNEW(arelent);
  reloc->sym_ptr_ptr = XNEW(asymbol*);
  *reloc->sym_ptr_ptr = symbol_get_bfdsym(fixp->fx_addsy);

  reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;
  reloc->howto = bfd_reloc_type_lookup(stdoutput, fixp->fx_r_type);
  reloc->addend = fixp->fx_offset;

  return reloc;
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
