#ifndef DCPU16_H
#define DCPU16_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DCPU16_SYMBOL_BYTE_SIZE 1024

#define DCPU16_INSTRUCTION_BYTE_SIZE 4
#define DCPU16_IMMEDIATE_BYTE_SIZE 4

#define DCPU16_VALUE_OF_MASK 0b0000001

enum dcpu16_value
{
  VAL_REG_A       = 0x00,
  VAL_REG_B       = 0x01,
  VAL_REG_C       = 0x02,
  VAL_REG_X       = 0x03,
  VAL_REG_Y       = 0x04,
  VAL_REG_Z       = 0x05,
  VAL_REG_I       = 0x06,
  VAL_REG_J       = 0x07,
  VAL_AT_REG      = 0x08 /* to 0x0F */, // [VAL_REG_*]
  VAL_AT_IMM_REG  = 0x10 /* to 0x17 */, // [VAL_REG_* + next word]
  VAL_PUSH_POP    = 0x18, // (PUSH / [--SP]) if in b, or (POP / [SP++]) if in a
  VAL_PEEK        = 0x19, // [SP] / PEEK
  VAL_PICK        = 0x1A, // [SP + next word] / PICK n
  VAL_SP          = 0x1B,
  VAL_PC          = 0x1C,
  VAL_EX          = 0x1D,
  VAL_AT_IMM      = 0x1E, // [next word]
  VAL_IMM         = 0x1F, // next word (literal)
  VAL_LIT         = 0x20 /* to 0x3F */, // literal value 0xffff-0x1e (-1..30) (only for a)
};

typedef struct 
{
  const char *name;
  uint8_t value;
} dcpu16_value_entry;

const dcpu16_value_entry dcpu16_value_map[] = 
{
  { "a",      VAL_REG_A },
  { "b",      VAL_REG_B },
  { "c",      VAL_REG_C },
  { "x",      VAL_REG_X },
  { "y",      VAL_REG_Y },
  { "z",      VAL_REG_Y },
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

enum dcpu16_basic_opcode
{
  BOP_SOP = 0x00, // Special Instruction
  BOP_SET = 0x01,
  BOP_ADD = 0x02,
  BOP_SUB = 0x03,
  BOP_MUL = 0x04,
  BOP_MLI = 0x05,
  BOP_DIV = 0x06,
  BOP_DVI = 0x07,
  BOP_MOD = 0x08,
  BOP_MDI = 0x09,
  BOP_AND = 0x0A,
  BOP_BOR = 0x0B,
  BOP_XOR = 0x0C,
  BOP_SHR = 0x0D,
  BOP_ASR = 0x0E,
  BOP_SHL = 0x0F,
  BOP_IFB = 0x10,
  BOP_IFC = 0x11,
  BOP_IFE = 0x12,
  BOP_IFN = 0x13,
  BOP_IFG = 0x14,
  BOP_IFA = 0x15,
  BOP_IFL = 0x16,
  BOP_IFU = 0x17,
  BOP_ADX = 0x1A,
  BOP_SBX = 0x1B,
  BOP_STI = 0x1E,
  BOP_STD = 0x1F,
};

enum dcpu16_special_opcode
{
  SOP_NIL = 0x00,
  SOP_JSR = 0x01,
  SOP_INT = 0x08,
  SOP_IAG = 0x09,
  SOP_IAS = 0x0A,
  SOP_RFI = 0x0B,
  SOP_IAQ = 0x0C,
  SOP_HWN = 0x10,
  SOP_HWQ = 0x11,
  SOP_HWI = 0x12
};

typedef struct
{
  const char *name;
  uint8_t bop;
  uint8_t sop; 
} dcpu16_op_entry;

const dcpu16_op_entry dcpu16_op_map[] = 
{
  { "set", BOP_SET, SOP_NIL },
  { "add", BOP_ADD, SOP_NIL },
  { "sub", BOP_SUB, SOP_NIL },
  { "mul", BOP_MUL, SOP_NIL },
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

typedef struct
{
  expressionS reg;
  expressionS imm;
} dcpu16_parse_argument;

typedef struct
{
  const dcpu16_op_entry *op;
  dcpu16_parse_argument lhs;
  dcpu16_parse_argument rhs;
} dcpu16_parse_instruction;

typedef struct
{
  uint8_t val;
  bool has_imm;
  uint16_t imm;
} dcpu16_build_argument;

typedef struct
{
  uint8_t op;
  dcpu16_build_argument b;
  dcpu16_build_argument a;
} dcpu16_build_instruction;

#ifdef __cplusplus
}
#endif

#endif /* DCPU16_H */
