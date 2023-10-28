#include "as.h"
#include "bfd.h"

const char *md_shortopts = "";
struct option md_longopts[] = {};
size_t md_longopts_size = sizeof(md_longopts);

// TODO: Comment chars and seperators
const char comment_chars[] = "";
const char line_comment_chars[] = "//";
const char line_separator_chars[] = ";";

// No floating point
const char EXP_CHARS[] = "";
const char FLT_CHARS[] = "";

// TODO: Pseudo ops
const pseudo_typeS md_pseudo_table[] =
{
  { (char*)0, (void(*)(int))0, 0 }
};

int md_parse_option(int c ATTRIBUTE_UNUSED, const char *arg ATTRIBUTE_UNUSED)
{
  // TODO: Parse
  return 0;
}

void md_show_usage(FILE *stream ATTRIBUTE_UNUSED)
{
  // TODO: CLI usage
  fprintf(stream, "\nDCPU16 options: none available yet\n");
}

void md_begin(void)
{
  // TODO: Initializer hook
  return;
}

void md_assemble(char *insn_str ATTRIBUTE_UNUSED)
{
  // TODO: Main assembler hook
  return;
}

symbolS *md_undefined_symbol(char *name ATTRIBUTE_UNUSED)
{
  // TODO: 'md_parse_name' fails
  return 0;
}

const char *md_atof(int type ATTRIBUTE_UNUSED, char *lit ATTRIBUTE_UNUSED, int *size ATTRIBUTE_UNUSED)
{
  // Not used
  return 0;
}

valueT md_section_align(asection *seg ATTRIBUTE_UNUSED, valueT val ATTRIBUTE_UNUSED)
{
  // TODO: Round up section
  return 0;
}

void md_convert_frag(bfd *abfd ATTRIBUTE_UNUSED, asection *seg ATTRIBUTE_UNUSED, fragS *fragp ATTRIBUTE_UNUSED)
{
  // Not used
  return;
}

void md_apply_fix(fixS *fixp ATTRIBUTE_UNUSED, valueT *val ATTRIBUTE_UNUSED, segT seg ATTRIBUTE_UNUSED)
{
  // Not used
  return;
}

arelent *tc_gen_reloc(asection *seg ATTRIBUTE_UNUSED, fixS *fixp ATTRIBUTE_UNUSED)
{
  // Not used
  return 0;
}

long md_pcrel_from(fixS *fixp ATTRIBUTE_UNUSED)
{
  as_fatal(_("unexpected call"));
  return 0;
}

int md_estimate_size_before_relax(fragS *fragp ATTRIBUTE_UNUSED, asection *seg ATTRIBUTE_UNUSED)
{
  as_fatal(_("unexpected call"));
  return 0;
}
