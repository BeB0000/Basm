#ifndef OPCODES_H
#define OPCODES_H

#include "beboasm.h"

// Extended Instruction Set
typedef enum {
    // Data Transfer
    OP_MOV   = 0x01,
    OP_MOVW  = 0x02,  // Move word
    OP_MOVB  = 0x03,  // Move byte
    OP_LOAD  = 0x04,
    OP_LOADB = 0x05,  // Load byte
    OP_LOADH = 0x06,  // Load halfword
    OP_STORE = 0x07,
    OP_STOREB = 0x08,
    OP_STOREH = 0x09,
    OP_PUSH  = 0x0A,
    OP_POP   = 0x0B,
    OP_PUSHA = 0x0C,  // Push all
    OP_POPA  = 0x0D,  // Pop all
    OP_XCHG  = 0x0E,  // Exchange
    OP_LEA   = 0x0F,  // Load effective address
    
    // Arithmetic
    OP_ADD   = 0x10,
    OP_ADDC  = 0x11,  // Add with carry
    OP_SUB   = 0x12,
    OP_SUBB  = 0x13,  // Subtract with borrow
    OP_MUL   = 0x14,
    OP_MULU  = 0x15,  // Multiply unsigned
    OP_DIV   = 0x16,
    OP_DIVU  = 0x17,  // Divide unsigned
    OP_MOD   = 0x18,
    OP_MODU  = 0x19,  // Modulo unsigned
    OP_INC   = 0x1A,
    OP_DEC   = 0x1B,
    OP_NEG   = 0x1C,  // Negate
    OP_ABS   = 0x1D,  // Absolute value
    
    // Extended Arithmetic
    OP_ADDI  = 0x1E,  // Add immediate
    OP_SUBI  = 0x1F,  // Subtract immediate
    OP_MULI  = 0x20,  // Multiply immediate
    OP_DIVI  = 0x21,  // Divide immediate
    
    // Logic
    OP_AND   = 0x30,
    OP_OR    = 0x31,
    OP_XOR   = 0x32,
    OP_NOT   = 0x33,
    OP_ANDI  = 0x34,  // AND immediate
    OP_ORI   = 0x35,  // OR immediate
    OP_XORI  = 0x36,  // XOR immediate
    OP_SHL   = 0x37,
    OP_SHR   = 0x38,
    OP_SHLA  = 0x39,  // Arithmetic shift left
    OP_SHRA  = 0x3A,  // Arithmetic shift right
    OP_ROL   = 0x3B,  // Rotate left
    OP_ROR   = 0x3C,  // Rotate right
    OP_CLR   = 0x3D,  // Clear
    OP_SETB  = 0x3E,  // Set bits (CHANGED from OP_SET)
    OP_TEST  = 0x3F,  // Test bits
    
    // Comparison
    OP_CMP   = 0x40,
    OP_CMPI  = 0x41,  // Compare immediate
    OP_TST   = 0x42,  // Test
    OP_CMN   = 0x43,  // Compare negative
    
    // Control Flow
    OP_JMP   = 0x50,
    OP_JZ    = 0x51,  // Jump if zero
    OP_JNZ   = 0x52,  // Jump if not zero
    OP_JE    = 0x53,
    OP_JNE   = 0x54,
    OP_JG    = 0x55,
    OP_JGE   = 0x56,
    OP_JL    = 0x57,
    OP_JLE   = 0x58,
    OP_JC    = 0x59,  // Jump if carry
    OP_JNC   = 0x5A,  // Jump if no carry
    OP_JO    = 0x5B,  // Jump if overflow
    OP_JNO   = 0x5C,  // Jump if no overflow
    OP_CALL  = 0x5D,
    OP_RET   = 0x5E,
    OP_RETI  = 0x5F,  // Return from interrupt
    OP_LOOP  = 0x60,  // Loop
    OP_SKIP  = 0x61,  // Skip next instruction
    
    // System
    OP_HALT  = 0x70,
    OP_NOP   = 0x71,
    OP_WAIT  = 0x72,
    OP_TRAP  = 0x73,  // Software trap
    OP_SVC   = 0x74,  // Supervisor call
    OP_IRET  = 0x75,  // Interrupt return
    
    // I/O
    OP_IN    = 0x80,
    OP_OUT   = 0x81,
    OP_INB   = 0x82,  // Input byte
    OP_OUTB  = 0x83,  // Output byte
    OP_INI   = 0x84,  // Input and increment
    OP_OUTI  = 0x85,  // Output and increment
    
    // String Operations
    OP_MOVS  = 0x90,  // Move string
    OP_CMPS  = 0x91,  // Compare string
    OP_SCAS  = 0x92,  // Scan string
    OP_LODS  = 0x93,  // Load string
    OP_STOS  = 0x94,  // Store string
    OP_REP   = 0x95,  // Repeat
    
    // Floating Point (optional)
    OP_FADD  = 0xA0,
    OP_FSUB  = 0xA1,
    OP_FMUL  = 0xA2,
    OP_FDIV  = 0xA3,
    OP_FCMP  = 0xA4,
    OP_FMOV  = 0xA5,
    
    // SIMD (optional)
    OP_VADD  = 0xB0,
    OP_VSUB  = 0xB1,
    OP_VMUL  = 0xB2,
    OP_VDOT  = 0xB3,  // Vector dot product
    
    // Debug
    OP_BREAK = 0xF0,
    OP_TRACE = 0xF1,
    
    // Pseudo Instructions
    OP_EQU   = 0xFE,
    OP_PSET  = 0xFF   // Pseudo SET (CHANGED from OP_SET)
} Opcode;

// Condition Codes
typedef enum {
    COND_ALWAYS = 0,  // Always execute
    COND_EQ     = 1,  // Equal
    COND_NE     = 2,  // Not equal
    COND_CS     = 3,  // Carry set
    COND_CC     = 4,  // Carry clear
    COND_MI     = 5,  // Negative
    COND_PL     = 6,  // Positive
    COND_VS     = 7,  // Overflow
    COND_VC     = 8,  // No overflow
    COND_HI     = 9,  // Unsigned higher
    COND_LS     = 10, // Unsigned lower or same
    COND_GE     = 11, // Signed greater or equal
    COND_LT     = 12, // Signed less than
    COND_GT     = 13, // Signed greater than
    COND_LE     = 14, // Signed less or equal
    COND_NV     = 15  // Never execute
} ConditionCode;

// Instruction Format Types
typedef enum {
    FORMAT_R,      // Register-register
    FORMAT_I,      // Immediate
    FORMAT_M,      // Memory
    FORMAT_B,      // Branch
    FORMAT_S,      // System
    FORMAT_V       // Vector
} InstructionFormat;

// Instruction Flags
typedef enum {
    IF_NONE       = 0x0000,
    IF_BRANCH     = 0x0001,  // Branch instruction
    IF_CALL       = 0x0002,  // Subroutine call
    IF_RETURN     = 0x0004,  // Return from subroutine
    IF_MEMORY     = 0x0008,  // Memory access
    IF_ARITH      = 0x0010,  // Arithmetic operation
    IF_LOGIC      = 0x0020,  // Logical operation
    IF_SHIFT      = 0x0040,  // Shift/rotate
    IF_COMPARE    = 0x0080,  // Compare/test
    IF_STACK      = 0x0100,  // Stack operation
    IF_IO         = 0x0200,  // I/O operation
    IF_PRIVILEGED = 0x0400,  // Privileged instruction
    IF_CONDITIONAL = 0x0800, // Conditional execution
    IF_ATOMIC     = 0x1000,  // Atomic operation
    IF_VECTOR     = 0x2000,  // Vector/SIMD operation
    IF_FLOAT      = 0x4000,  // Floating point
    IF_PSEUDO     = 0x8000   // Pseudo-instruction
} InstructionFlags;

// Operand Types
typedef enum {
    OT_NONE = 0,
    OT_REG  = 1 << 0,     // Register
    OT_IMM  = 1 << 1,     // Immediate
    OT_MEM  = 1 << 2,     // Memory
    OT_LABEL = 1 << 3,    // Label
    OT_STRING = 1 << 4,   // String
    OT_EXPR = 1 << 5,     // Expression
    OT_BIT  = 1 << 6,     // Bit position
    OT_REL  = 1 << 7      // Relative address
} OperandType;

// Opcode Metadata Structure
typedef struct {
    char mnemonic[16];
    Opcode opcode;
    uint8_t format;
    uint8_t size;           // Base size in bytes
    uint8_t cycles;         // Base cycles
    uint8_t operands;       // Number of operands
    struct {
        uint8_t types;      // Allowed operand types (bitmask of OperandType)
        char name[16];      // Operand name for error messages
    } operand_specs[4];
    uint16_t flags;         // Instruction flags
    char description[64];
} OpcodeMetadata;

// Forward declaration of Instruction structure
typedef struct {
    char label[MAX_LABEL_LEN];
    OpcodeMetadata *info;
    Operand operands[MAX_OPERANDS];
    int operand_count;
    uint32_t address;
    uint8_t encoding[8];    // Maximum instruction size
    int encoding_size;
    ConditionCode condition;
} Instruction;

// Parse results
#define PARSE_ERROR       0
#define PARSE_DIRECTIVE   1
#define PARSE_INSTRUCTION 2
#define PARSE_MACRO       3
#define PARSE_LABEL       4
#define PARSE_EMPTY       5

// Symbol types
#define SYM_CODE     0x01
#define SYM_DATA     0x02
#define SYM_BSS      0x04
#define SYM_EXTERN   0x08
#define SYM_MACRO    0x10
#define SYM_EQU      0x20
#define SYM_ABSOLUTE 0x40

// Scope
#define SCOPE_LOCAL  0
#define SCOPE_GLOBAL 1
#define SCOPE_WEAK   2

// Instruction Set Table (extern - defined in assembler.c)
extern OpcodeMetadata instruction_set[];

// Helper Functions
OpcodeMetadata* opcode_find(const char *mnemonic);
OpcodeMetadata* opcode_by_value(Opcode opcode);
bool opcode_validate_operands(OpcodeMetadata *meta, Operand operands[], int count);
int opcode_calculate_size(OpcodeMetadata *meta, Operand operands[], int count);
const char* condition_to_str(ConditionCode cond);
ConditionCode str_to_condition(const char *str);
uint8_t encode_operand(Operand *op, int operand_index);
int decode_operand(uint8_t encoding, Operand *op);

// Instruction encoding/decoding
uint32_t encode_instruction(Instruction *inst, uint8_t *buffer);
int decode_instruction(uint8_t *buffer, Instruction *inst, uint32_t address);

// Size calculation
int instruction_size(InstructionInfo *info);
int instruction_full_size(Instruction *inst);

// Register utilities
bool is_valid_register(int reg_num);
bool is_special_register(int reg_num);
const char* register_to_name(int reg_num);
int name_to_register(const char *name);

// Addressing mode utilities
bool is_valid_addressing_mode(AddressingMode mode, OpcodeMetadata *meta, int operand_index);
bool is_memory_addressing(AddressingMode mode);
bool is_immediate_addressing(AddressingMode mode);
bool is_register_addressing(AddressingMode mode);

#endif // OPCODES_H