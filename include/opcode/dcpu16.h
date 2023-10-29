#ifndef DCPU16_H
#define DCPU16_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DCPU16_SYMBOL_BYTE_SIZE 1024

#define DCPU16_WORD_BYTE_SIZE 2
#define DCPU16_INSTRUCTION_BYTE_SIZE DCPU16_WORD_BYTE_SIZE
#define DCPU16_IMMEDIATE_BYTE_SIZE DCPU16_WORD_BYTE_SIZE

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

extern const char *dcpu16_value_names[];

typedef struct 
{
  const char *name;
  uint8_t value;
} dcpu16_value_entry;

extern const dcpu16_value_entry dcpu16_value_map[];
extern const size_t dcpu16_value_map_size;

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

extern const char *dcpu16_bop_names[];

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

extern const char *dcpu16_sop_names[];

typedef struct
{
  const char *name;
  uint8_t bop;
  uint8_t sop; 
} dcpu16_op_entry;

extern const dcpu16_op_entry dcpu16_op_map[];
extern const size_t dcpu16_op_map_size;

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
