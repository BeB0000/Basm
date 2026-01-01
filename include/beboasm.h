#ifndef BEBOASM_H
#define BEBOASM_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// ==========================================
// Constants and Configuration
// ==========================================
#define MAX_LABEL_LEN          64
#define MAX_MNEMONIC_LEN       16
#define MAX_OPERANDS           4
#define MAX_SYMBOLS            4096
#define MEMORY_SIZE            16777216   // 16MB
#define STACK_SIZE             4096
#define NUM_REGISTERS          32         // Extended from 16 to 32
#define MAX_INCLUDE_DEPTH      16
#define MAX_MACRO_PARAMS       8
#define CACHE_SIZE             256

// Special Purpose Registers
enum {
    REG_PC = 28,    // Program Counter
    REG_SP = 29,    // Stack Pointer
    REG_FP = 30,    // Frame Pointer
    REG_LR = 31     // Link Register
};

// Flags for condition codes
typedef enum {
    FLAG_ZERO      = 0x01,
    FLAG_CARRY     = 0x02,
    FLAG_OVERFLOW  = 0x04,
    FLAG_NEGATIVE  = 0x08,
    FLAG_INTERRUPT = 0x10,
    FLAG_DECIMAL   = 0x20,
    FLAG_BREAK     = 0x40,
    FLAG_DEBUG     = 0x80
} ProcessorFlags;

// Addressing Modes
typedef enum {
    AM_IMMEDIATE,      // #value
    AM_DIRECT,         // [address]
    AM_REGISTER,       // R0-R31
    AM_REGISTER_INDIRECT,  // [R1]
    AM_REGISTER_INDEXED,   // [R1 + R2]
    AM_DISPLACEMENT,   // [R1 + #10]
    AM_PC_RELATIVE,    // label
    AM_STACK,          // PUSH/POP
    AM_ABSOLUTE        // direct address
} AddressingMode;

// Operand Structure
typedef struct {
    AddressingMode mode;
    union {
        int32_t immediate;
        uint16_t address;
        uint8_t reg_num;
        struct {
            uint8_t base_reg;
            uint8_t index_reg;
            int16_t displacement;
        } indexed;
    } value;
    char label[MAX_LABEL_LEN];
    bool resolved;
} Operand;

// Instruction Encoding
typedef struct {
    uint8_t opcode;
    uint8_t cond;          // Condition code
    uint8_t mode;          // Addressing mode
    uint8_t reg_dest;      // Destination register
    uint8_t reg_src1;      // Source register 1
    uint8_t reg_src2;      // Source register 2
    union {
        int16_t imm16;
        uint16_t addr16;
        int32_t imm32;
    } immediate;
    uint8_t size;          // Instruction size in bytes
} InstructionEncoding;

// Instruction Metadata
typedef struct {
    char mnemonic[MAX_MNEMONIC_LEN];
    uint8_t opcode;
    uint8_t size;           // Instruction size in bytes
    uint8_t cycles;         // Estimated clock cycles
    uint8_t operands;       // Number of operands
    struct {
        uint8_t allowed_modes;
        uint8_t type;       // REG, IMM, MEM, LABEL
        char description[32];
    } operand_info[MAX_OPERANDS];
} InstructionInfo;

// Symbol with enhanced information
typedef struct {
    char name[MAX_LABEL_LEN];
    uint32_t value;
    uint32_t size;
    uint8_t type;           // CODE, DATA, BSS, EXTERNAL, MACRO
    uint8_t scope;          // LOCAL, GLOBAL, WEAK
    uint16_t section;
    bool defined;
    bool exported;
    int line;
    char file[256];
} Symbol;

// Section Information
typedef struct {
    char name[32];
    uint32_t address;
    uint32_t size;
    uint8_t attributes;     // READ, WRITE, EXECUTE
    uint8_t *data;
} Section;

// Macro Definition
typedef struct {
    char name[MAX_LABEL_LEN];
    char params[MAX_MACRO_PARAMS][MAX_LABEL_LEN];
    int param_count;
    char **body;
    int body_lines;
    int line;
    char file[256];
} Macro;

// Assembler State with enhanced features
typedef struct {
    // Core Assembler State
    Section *sections;
    int section_count;
    int current_section;
    
    Symbol *symbols;
    int symbol_count;
    
    Macro *macros;
    int macro_count;
    
    uint8_t *memory;
    uint32_t pc;
    uint32_t org;           // Current origin
    
    // File Management
    FILE *include_stack[MAX_INCLUDE_DEPTH];
    int include_depth;
    char current_file[256];
    int current_line;
    
    // Error/Warning System
    struct {
        int errors;
        int warnings;
        char messages[256][256];
        int message_count;
    } diagnostics;
    
    // Optimization
    bool optimize;
    int optimization_level;
    
    // Debug Information
    bool debug;
    char *debug_info;
    
    // Listing Generation
    FILE *list_file;
    bool generate_listing;
    
    // Relocation Information
    struct {
        uint32_t address;
        Symbol *symbol;
        uint8_t type;
    } *relocations;
    int relocation_count;
    
    // Assembly Directives
    struct {
        bool in_data_section;
        bool in_code_section;
        bool in_macro;
        uint32_t fill_value;
    } state;
    
    // Cache for Performance
    struct {
        Symbol *symbol_cache[CACHE_SIZE];
        int cache_hits;
        int cache_misses;
    } cache;
} AssemblerState;

// Simulator State
typedef struct {
    uint32_t registers[NUM_REGISTERS];
    uint32_t flags;
    uint8_t *memory;
    uint32_t pc;
    uint32_t sp;
    uint32_t fp;
    
    // Statistics
    uint64_t instructions_executed;
    uint64_t clock_cycles;
    uint64_t memory_accesses;
    
    // Breakpoints
    uint32_t breakpoints[256];
    int breakpoint_count;
    
    // Watchpoints
    struct {
        uint32_t address;
        uint8_t size;
        char watch_type; // 'r', 'w', 'x'
    } watchpoints[256];
    int watchpoint_count;
    
    // Pipeline (for advanced simulation)
    struct {
        uint32_t fetch;
        uint32_t decode;
        uint32_t execute;
        uint32_t writeback;
        bool stalled;
    } pipeline;
    
    // I/O Ports
    uint8_t io_ports[256];
    
    // Interrupt Controller
    struct {
        bool enabled;
        uint8_t mask;
        uint8_t pending;
        void (*handlers[16])(void*);
    } interrupt;
    
    // Debug Interface
    bool single_step;
    bool trace;
    FILE *trace_file;
    
    // Execution Control
    bool running;
    bool halted;
} SimulatorState;

// ==========================================
// Function Prototypes
// ==========================================

// Assembler Lifecycle
AssemblerState* assembler_create(bool optimize, bool debug);
void assembler_destroy(AssemblerState *state);
void assembler_reset(AssemblerState *state);

// Assembly Process
int assemble_file(AssemblerState *state, const char *filename);
int assemble_string(AssemblerState *state, const char *code);
void assemble_pass(AssemblerState *state, int pass);

// Symbol Management
Symbol* symbol_add(AssemblerState *state, const char *name, uint32_t value, uint8_t type);
Symbol* symbol_find(AssemblerState *state, const char *name);
void symbol_export(AssemblerState *state, const char *name);
void symbol_set_scope(AssemblerState *state, const char *name, uint8_t scope);

// Section Management
Section* section_create(AssemblerState *state, const char *name, uint32_t address, uint8_t attrs);
Section* section_find(AssemblerState *state, const char *name);
void section_switch(AssemblerState *state, const char *name);

// Macro Processing
Macro* macro_define(AssemblerState *state, const char *name, const char *params);
int macro_expand(AssemblerState *state, Macro *macro, const char *args[]);
Macro* macro_find(AssemblerState *state, const char *name);

// Error Handling
void error_add(AssemblerState *state, const char *format, ...);
void warning_add(AssemblerState *state, const char *format, ...);
void print_diagnostics(AssemblerState *state);

// Output Generation
int write_binary(AssemblerState *state, const char *filename);
int write_hex(AssemblerState *state, const char *filename);
int write_srec(AssemblerState *state, const char *filename);
int write_listing(AssemblerState *state, const char *filename);
int write_map(AssemblerState *state, const char *filename);

// Optimization
void optimize_instructions(AssemblerState *state);
void peephole_optimize(AssemblerState *state);
void constant_folding(AssemblerState *state);
void dead_code_elimination(AssemblerState *state);

// Debug Information
void debug_add_info(AssemblerState *state, const char *info);
void generate_debug_info(AssemblerState *state);

// Simulator Functions
SimulatorState* simulator_create(AssemblerState *state);
void simulator_destroy(SimulatorState *sim);
int simulator_load(SimulatorState *sim, const char *filename);
int simulator_run(SimulatorState *sim);
int simulator_step(SimulatorState *sim);
void simulator_reset(SimulatorState *sim);

// Debugger Functions
void debugger_start(SimulatorState *sim);
void debugger_add_breakpoint(SimulatorState *sim, uint32_t address);
void debugger_remove_breakpoint(SimulatorState *sim, uint32_t address);
void debugger_add_watchpoint(SimulatorState *sim, uint32_t address, char type);
void debugger_print_registers(SimulatorState *sim);
void debugger_print_memory(SimulatorState *sim, uint32_t address, uint32_t size);
void debugger_disassemble(SimulatorState *sim, uint32_t address, uint32_t count);

// Memory Access Functions
uint8_t memory_read_byte(SimulatorState *sim, uint32_t address);
uint16_t memory_read_word(SimulatorState *sim, uint32_t address);
void memory_write_byte(SimulatorState *sim, uint32_t address, uint8_t value);
void memory_write_word(SimulatorState *sim, uint32_t address, uint16_t value);

// Utility Functions
uint32_t parse_number(const char *str);
int parse_register(const char *str);
int parse_addressing_mode(const char *str, Operand *operand);
char* addressing_mode_to_str(AddressingMode mode);
char* register_name(int reg_num);
int instruction_size(InstructionInfo *info);
uint32_t calculate_displacement(uint32_t from, uint32_t to);

// String Pool (for memory efficiency)
char* string_pool_add(const char *str);
char* string_pool_find(const char *str);
void string_pool_clear(void);

// Hash Table for Fast Lookup
typedef struct HashTable HashTable;
HashTable* hash_table_create(size_t size);
void* hash_table_get(HashTable *ht, const char *key);
void hash_table_put(HashTable *ht, const char *key, void *value);
void hash_table_remove(HashTable *ht, const char *key);

#endif // BEBOASM_H