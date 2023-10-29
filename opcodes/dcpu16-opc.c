#include "sysdep.h"
#include "opcode/dcpu16.h"

const char *dcpu16_value_names[] = {
    "A",
    "B",
    "C",
    "X",
    "Y",
    "Z",
    "I",
    "J",

    "A",
    "B",
    "C",
    "X",
    "Y",
    "Z",
    "I",
    "J",

    "A",
    "B",
    "C",
    "X",
    "Y",
    "Z",
    "I",
    "J",

    "PUSH/POP",
    "PEEK",
    "PICK",
    "SP",
    "PC",
    "EX"
};

const dcpu16_value_entry dcpu16_value_map[] = 
{
  { "a",      VAL_REG_A },
  { "b",      VAL_REG_B },
  { "c",      VAL_REG_C },
  { "x",      VAL_REG_X },
  { "y",      VAL_REG_Y },
  { "z",      VAL_REG_Z },
  { "i",      VAL_REG_I },
  { "j",      VAL_REG_J },
  { "sp",     VAL_SP },
  { "push",   VAL_PUSH_POP },
  { "pop",    VAL_PUSH_POP },
  { "peek",   VAL_PEEK },
  { "pc",     VAL_PC },
  { "ex",     VAL_EX }
};
const size_t dcpu16_value_map_size = sizeof(dcpu16_value_map) / sizeof(dcpu16_value_map[0]);

const char *dcpu16_bop_names[] =
{
  0,
  "SET",
  "ADD",
  "SUB",
  "MUL",
  "MLI",
  "DIV",
  "DVI",
  "MOD",
  "MDI",
  "AND",
  "BOR",
  "XOR",
  "SHR",
  "ASR",
  "SHL",
  "IFB",
  "IFC",
  "IFE",
  "IFN",
  "IFG",
  "IFA",
  "IFL",
  "IFU",
  0,
  0,
  "ADX",
  "SBX",
  0,
  0,
  "STI",
  "STD"
};

const char *dcpu16_sop_names[] =
{
  0,
  "JSR",
  0,
  0,
  0,
  0,
  0,
  0,
  "INT",
  "IAG",
  "IAS",
  "RFI",
  "IAQ",
  0,
  0,
  0,
  "HWN",
  "HWQ",
  "HWI"
};

const dcpu16_op_entry dcpu16_op_map[] = 
{
  { "set", BOP_SET, SOP_NIL },
  { "add", BOP_ADD, SOP_NIL },
  { "sub", BOP_SUB, SOP_NIL },
  { "mul", BOP_MUL, SOP_NIL },
  { "mli", BOP_MLI, SOP_NIL },
  { "div", BOP_DIV, SOP_NIL },
  { "dvi", BOP_DVI, SOP_NIL },
  { "mod", BOP_MOD, SOP_NIL },
  { "mdi", BOP_MDI, SOP_NIL },
  { "and", BOP_AND, SOP_NIL },
  { "bor", BOP_BOR, SOP_NIL },
  { "xor", BOP_XOR, SOP_NIL },
  { "shr", BOP_SHR, SOP_NIL },
  { "asr", BOP_ASR, SOP_NIL },
  { "shl", BOP_SHL, SOP_NIL },
  { "ifb", BOP_IFB, SOP_NIL },
  { "ifc", BOP_IFC, SOP_NIL },
  { "ife", BOP_IFE, SOP_NIL },
  { "ifn", BOP_IFN, SOP_NIL },
  { "ifg", BOP_IFG, SOP_NIL },
  { "ifa", BOP_IFA, SOP_NIL },
  { "ifl", BOP_IFL, SOP_NIL },
  { "ifu", BOP_IFU, SOP_NIL },
  { "adx", BOP_ADX, SOP_NIL },
  { "sbx", BOP_SBX, SOP_NIL },
  { "sti", BOP_STI, SOP_NIL },
  { "std", BOP_STD, SOP_NIL },

  { "jsr", BOP_SOP, SOP_JSR },
  { "int", BOP_SOP, SOP_INT },
  { "iag", BOP_SOP, SOP_IAG },
  { "ias", BOP_SOP, SOP_IAS },
  { "rfi", BOP_SOP, SOP_RFI },
  { "iaq", BOP_SOP, SOP_IAQ },
  { "hwn", BOP_SOP, SOP_HWN },
  { "hwq", BOP_SOP, SOP_HWQ },
  { "hwi", BOP_SOP, SOP_HWI },
};
const size_t dcpu16_op_map_size = sizeof(dcpu16_op_map) / sizeof(dcpu16_op_map[0]);
