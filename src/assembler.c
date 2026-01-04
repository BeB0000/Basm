#include "../include/beboasm.h"
#include "../include/opcodes.h"
#include <stdarg.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

// Forward declarations for functions defined elsewhere
void resolve_references(AssemblerState *state);
void handle_include(AssemblerState *state, const char *filename, int pass);
int parse_line(AssemblerState *state, char *line, Instruction *inst, int pass);
void emit_instruction(AssemblerState *state, Instruction *inst, int pass);
void handle_directive(AssemblerState *state, char *line, int pass);
uint32_t parse_number(const char *str);
void emit_byte(AssemblerState *state, uint8_t byte);
void optimize_instructions(AssemblerState *state);
uint32_t hash_string(const char *str);
char *string_pool_add(const char *str);
int instruction_size(InstructionInfo *info);
bool parse_operand(AssemblerState *state, char *str, Operand *operand);

// String Pool for efficient memory usage
static char *string_pool[4096];
static int string_pool_count = 0;

// Global instruction set (Updated with corrected OP_SETB)
OpcodeMetadata instruction_set[] = {
    // Data Transfer
    {"MOV", OP_MOV, FORMAT_R, 2, 1, 2, {{OT_REG | OT_MEM, "dst"}, {OT_REG | OT_IMM | OT_MEM, "src"}}, IF_NONE, "Move data"},
    {"MOVW", OP_MOVW, FORMAT_R, 2, 1, 2, {{OT_REG, "dst"}, {OT_REG | OT_IMM, "src"}}, IF_NONE, "Move 32-bit data"},
    {"LOAD", OP_LOAD, FORMAT_M, 3, 2, 2, {{OT_REG, "reg"}, {OT_MEM | OT_LABEL, "addr"}}, IF_MEMORY, "Load from memory"},
    {"LDB", OP_LOADB, FORMAT_M, 3, 2, 2, {{OT_REG, "reg"}, {OT_MEM | OT_LABEL, "addr"}}, IF_MEMORY, "Load byte from memory"},
    {"LDW", OP_LOADH, FORMAT_M, 3, 2, 2, {{OT_REG, "reg"}, {OT_MEM | OT_LABEL, "addr"}}, IF_MEMORY, "Load word from memory"},
    {"STORE", OP_STORE, FORMAT_M, 3, 2, 2, {{OT_REG, "reg"}, {OT_MEM | OT_LABEL, "addr"}}, IF_MEMORY, "Store to memory"},
    {"STB", OP_STOREB, FORMAT_M, 3, 2, 2, {{OT_REG, "reg"}, {OT_MEM | OT_LABEL, "addr"}}, IF_MEMORY, "Store byte to memory"},
    {"STW", OP_STOREH, FORMAT_M, 3, 2, 2, {{OT_REG, "reg"}, {OT_MEM | OT_LABEL, "addr"}}, IF_MEMORY, "Store word to memory"},
    {"PUSH", OP_PUSH, FORMAT_R, 1, 1, 1, {{OT_REG | OT_IMM, "src"}}, IF_STACK, "Push to stack"},
    {"POP", OP_POP, FORMAT_R, 1, 1, 1, {{OT_REG, "dst"}}, IF_STACK, "Pop from stack"},
    {"XCHG", OP_XCHG, FORMAT_R, 2, 2, 2, {{OT_REG, "dst"}, {OT_REG, "src"}}, IF_NONE, "Exchange registers"},
    
    // Arithmetic
    {"ADD", OP_ADD, FORMAT_R, 2, 1, 3, {{OT_REG, "dst"}, {OT_REG | OT_IMM, "src1"}, {OT_REG | OT_IMM, "src2"}}, IF_ARITH, "Add"},
    {"SUB", OP_SUB, FORMAT_R, 2, 1, 3, {{OT_REG, "dst"}, {OT_REG | OT_IMM, "src1"}, {OT_REG | OT_IMM, "src2"}}, IF_ARITH, "Subtract"},
    {"MUL", OP_MUL, FORMAT_R, 2, 3, 3, {{OT_REG, "dst"}, {OT_REG | OT_IMM, "src1"}, {OT_REG | OT_IMM, "src2"}}, IF_ARITH, "Multiply"},
    {"DIV", OP_DIV, FORMAT_R, 2, 4, 3, {{OT_REG, "dst"}, {OT_REG | OT_IMM, "src1"}, {OT_REG | OT_IMM, "src2"}}, IF_ARITH, "Divide"},
    {"MOD", OP_MOD, FORMAT_R, 2, 4, 3, {{OT_REG, "dst"}, {OT_REG | OT_IMM, "src1"}, {OT_REG | OT_IMM, "src2"}}, IF_ARITH, "Remainder"},
    {"INC", OP_INC, FORMAT_R, 1, 1, 1, {{OT_REG, "dst"}}, IF_ARITH, "Increment"},
    {"DEC", OP_DEC, FORMAT_R, 1, 1, 1, {{OT_REG, "dst"}}, IF_ARITH, "Decrement"},
    
    // Logic (Note: OP_SET changed to OP_SETB)
    {"AND", OP_AND, FORMAT_R, 2, 1, 3, {{OT_REG, "dst"}, {OT_REG | OT_IMM, "src1"}, {OT_REG | OT_IMM, "src2"}}, IF_LOGIC, "Logical AND"},
    {"OR", OP_OR, FORMAT_R, 2, 1, 3, {{OT_REG, "dst"}, {OT_REG | OT_IMM, "src1"}, {OT_REG | OT_IMM, "src2"}}, IF_LOGIC, "Logical OR"},
    {"XOR", OP_XOR, FORMAT_R, 2, 1, 3, {{OT_REG, "dst"}, {OT_REG | OT_IMM, "src1"}, {OT_REG | OT_IMM, "src2"}}, IF_LOGIC, "Logical XOR"},
    {"NOT", OP_NOT, FORMAT_R, 1, 1, 1, {{OT_REG, "dst"}}, IF_LOGIC, "Logical NOT"},
    {"SHL", OP_SHL, FORMAT_R, 2, 1, 2, {{OT_REG, "dst"}, {OT_REG | OT_IMM, "shift"}}, IF_SHIFT, "Shift left"},
    {"SHR", OP_SHR, FORMAT_R, 2, 1, 2, {{OT_REG, "dst"}, {OT_REG | OT_IMM, "shift"}}, IF_SHIFT, "Shift right"},
    {"CLR", OP_CLR, FORMAT_R, 1, 1, 1, {{OT_REG, "dst"}}, IF_LOGIC, "Clear register"},
    {"SETB", OP_SETB, FORMAT_R, 2, 1, 2, {{OT_REG, "dst"}, {OT_REG | OT_IMM, "mask"}}, IF_LOGIC, "Set bits"}, // Fixed: OP_SET -> OP_SETB
    
    // Comparison
    {"CMP", OP_CMP, FORMAT_R, 2, 1, 2, {{OT_REG | OT_IMM, "src1"}, {OT_REG | OT_IMM, "src2"}}, IF_COMPARE, "Compare"},
    {"TEST", OP_TEST, FORMAT_R, 2, 1, 2, {{OT_REG, "src1"}, {OT_REG | OT_IMM, "src2"}}, IF_COMPARE, "Test bits"},
    
    // Control Flow
    {"JMP", OP_JMP, FORMAT_B, 3, 2, 1, {{OT_LABEL, "target"}}, IF_BRANCH, "Unconditional jump"},
    {"JE", OP_JE, FORMAT_B, 3, 2, 1, {{OT_LABEL, "target"}}, IF_BRANCH | IF_CONDITIONAL, "Jump if equal"},
    {"JNE", OP_JNE, FORMAT_B, 3, 2, 1, {{OT_LABEL, "target"}}, IF_BRANCH | IF_CONDITIONAL, "Jump if not equal"},
    {"JG", OP_JG, FORMAT_B, 3, 2, 1, {{OT_LABEL, "target"}}, IF_BRANCH | IF_CONDITIONAL, "Jump if greater"},
    {"JL", OP_JL, FORMAT_B, 3, 2, 1, {{OT_LABEL, "target"}}, IF_BRANCH | IF_CONDITIONAL, "Jump if less"},
    {"JGE", OP_JGE, FORMAT_B, 3, 2, 1, {{OT_LABEL, "target"}}, IF_BRANCH | IF_CONDITIONAL, "Jump if greater or equal"},
    {"JLE", OP_JLE, FORMAT_B, 3, 2, 1, {{OT_LABEL, "target"}}, IF_BRANCH | IF_CONDITIONAL, "Jump if less or equal"},
    {"CALL", OP_CALL, FORMAT_B, 3, 3, 1, {{OT_LABEL, "target"}}, IF_CALL, "Call subroutine"},
    {"RET", OP_RET, FORMAT_S, 1, 2, 0, {{0, ""}}, IF_RETURN, "Return from subroutine"},
    
    // System
    {"HALT", OP_HALT, FORMAT_S, 1, 1, 0, {{0, ""}}, IF_NONE, "Halt processor"},
    {"NOP", OP_NOP, FORMAT_S, 1, 1, 0, {{0, ""}}, IF_NONE, "No operation"},
    
    // I/O
    {"IN", OP_IN, FORMAT_I, 2, 2, 2, {{OT_REG, "reg"}, {OT_IMM, "port"}}, IF_IO, "Input from port"},
    {"OUT", OP_OUT, FORMAT_I, 2, 2, 2, {{OT_IMM, "port"}, {OT_REG, "reg"}}, IF_IO, "Output to port"},
    
    // Terminator
    {"", 0, 0, 0, 0, 0, {{0, ""}}, 0, ""}
};

AssemblerState* assembler_create(bool optimize, bool debug) {
    AssemblerState *state = calloc(1, sizeof(AssemblerState));
    if (!state) return NULL;
    
    // Allocate memory
    state->memory = calloc(MEMORY_SIZE, 1);
    state->sections = calloc(16, sizeof(Section));
    state->symbols = calloc(MAX_SYMBOLS, sizeof(Symbol));
    state->macros = calloc(256, sizeof(Macro));
    state->relocations = calloc(1024, sizeof(*state->relocations));
    
    // Initialize defaults
    state->optimize = optimize;
    state->debug = debug;
    state->optimization_level = optimize ? 2 : 0;
    
    // Create default sections
    section_create(state, ".text", 0x0000, 0x05); // Read + Execute
    section_create(state, ".data", 0x4000, 0x06); // Read + Write (Moved to avoid collision at 0x1000)
    section_create(state, ".bss", 0x6000, 0x06);  // Read + Write
    section_create(state, ".stack", 0x8000, 0x06); // Read + Write
    
    section_switch(state, ".text");
    
    // Initialize diagnostics
    state->diagnostics.errors = 0;
    state->diagnostics.warnings = 0;
    
    return state;
}

void assembler_destroy(AssemblerState *state) {
    if (!state) return;
    
    // Free sections
    for (int i = 0; i < state->section_count; i++) {
        if (state->sections[i].data) {
            free(state->sections[i].data);
        }
    }
    
    // Close include files
    for (int i = 0; i < state->include_depth; i++) {
        if (state->include_stack[i]) {
            fclose(state->include_stack[i]);
        }
    }
    
    // Close listing file
    if (state->list_file) {
        fclose(state->list_file);
    }
    
    // Free debug info
    if (state->debug_info) {
        free(state->debug_info);
    }
    
    // Free string pool
    for (int i = 0; i < string_pool_count; i++) {
        free(string_pool[i]);
    }
    
    // Free allocations
    free(state->memory);
    free(state->sections);
    free(state->symbols);
    free(state->macros);
    free(state->relocations);
    free(state);
}

int assemble_file(AssemblerState *state, const char *filename) {
    if (!state || !filename) return 0;
    
    printf("Assembling: %s\n", filename);
    
    // Store current file info
    strncpy(state->current_file, filename, sizeof(state->current_file) - 1);
    state->current_file[sizeof(state->current_file) - 1] = '\0';
    state->current_line = 0;
    
    // Open file
    FILE *file = fopen(filename, "r");
    if (!file) {
        error_add(state, "Cannot open file: %s", filename);
        return 0;
    }
    
    // Push to include stack
    if (state->include_depth >= MAX_INCLUDE_DEPTH) {
        error_add(state, "Include depth too deep");
        fclose(file);
        return 0;
    }
    state->include_stack[state->include_depth++] = file;
    
    // First pass: collect symbols and macros
    printf("Pass 1: Collecting symbols...\n");
    assemble_pass(state, 1);
    
    // Second pass: generate code
    printf("Pass 2: Generating code...\n");
    rewind(file);
    state->current_line = 0;
    section_switch(state, ".text");
    assemble_pass(state, 2);
    
    // Third pass: resolve references
    printf("Pass 3: Resolving references...\n");
    resolve_references(state);
    
    // Optimize if requested
    if (state->optimize) {
        printf("Optimizing code...\n");
        optimize_instructions(state);
    }
    
    // Pop from include stack
    state->include_depth--;
    fclose(file);
    
    // Print diagnostics
    print_diagnostics(state);
    
    return state->diagnostics.errors == 0;
}

void assemble_pass(AssemblerState *state, int pass) {
    char line[1024];
    char original_line[1024];
    
    while (fgets(line, sizeof(line), state->include_stack[state->include_depth-1])) {
        state->current_line++;
        strcpy(original_line, line);
        
        // Remove newline
        line[strcspn(line, "\n")] = '\0';
        
        // Skip empty lines and comments
        if (line[0] == '\0' || line[0] == ';') {
            continue;
        }
        
        // Handle include directive
        if (strncmp(line, ".include", 8) == 0 || strncmp(line, ".INCLUDE", 8) == 0) {
            char include_file[256];
            if (sscanf(line + 8, " \"%255[^\"]\"", include_file) == 1 ||
                sscanf(line + 8, " <%255[^>]>", include_file) == 1) {
                handle_include(state, include_file, pass);
            }
            continue;
        }
        
        // Parse the line
        Instruction inst;
        memset(&inst, 0, sizeof(inst));
        
        int result = parse_line(state, line, &inst, pass);
        
        if (result == PARSE_ERROR) {
            error_add(state, "Line %d: Syntax error", state->current_line);
        } else {
             // Handle label for both directives and instructions
             if (pass == 1 && inst.label[0]) {
                 symbol_add(state, inst.label, state->pc, SYM_CODE);
             }

             if (result == PARSE_DIRECTIVE) {
                // Use original_line because parse_line modifies 'line' (cuts at colon)
                // We need to find where the directive starts
                char *dir_start = original_line;
                char *colon = strchr(original_line, ':');
                if (colon) {
                    dir_start = colon + 1;
                }
                handle_directive(state, dir_start, pass);
             } else if (result == PARSE_INSTRUCTION) {
                if (pass == 1) {
                    // Update PC with instruction size
                    state->pc += instruction_full_size(&inst);
                } else if (pass == 2) {
                    // Emit instruction
                    emit_instruction(state, &inst, pass);
                }
             }
        }
    }
}

Symbol* symbol_add(AssemblerState *state, const char *name, uint32_t value, uint8_t type) {
    if (state->symbol_count >= MAX_SYMBOLS) {
        error_add(state, "Symbol table full");
        return NULL;
    }
    
    Symbol *sym = &state->symbols[state->symbol_count++];
    
    // Copy name (using string pool for efficiency)
    char *pooled_name = string_pool_add(name);
    if (pooled_name) {
        strncpy(sym->name, pooled_name, sizeof(sym->name) - 1);
    } else {
        strncpy(sym->name, name, sizeof(sym->name) - 1);
    }
    sym->name[sizeof(sym->name) - 1] = '\0';
    
    sym->value = value;
    sym->type = type;
    sym->defined = 1;
    sym->scope = SCOPE_LOCAL;
    sym->section = state->current_section;
    sym->line = state->current_line;
    strncpy(sym->file, state->current_file, sizeof(sym->file) - 1);
    sym->file[sizeof(sym->file) - 1] = '\0';
    
    // Add to cache
    uint32_t hash = hash_string(name) % CACHE_SIZE;
    state->cache.symbol_cache[hash] = sym;
    
    return sym;
}

Symbol* symbol_find(AssemblerState *state, const char *name) {
    // Check cache first
    uint32_t hash = hash_string(name) % CACHE_SIZE;
    Symbol *cached = state->cache.symbol_cache[hash];
    if (cached && strcmp(cached->name, name) == 0) {
        state->cache.cache_hits++;
        return cached;
    }
    
    // Linear search
    for (int i = 0; i < state->symbol_count; i++) {
        if (strcmp(state->symbols[i].name, name) == 0) {
            state->cache.cache_misses++;
            // Update cache
            state->cache.symbol_cache[hash] = &state->symbols[i];
            return &state->symbols[i];
        }
    }
    
    return NULL;
}

Section* section_create(AssemblerState *state, const char *name, uint32_t address, uint8_t attrs) {
    if (state->section_count >= 16) {
        error_add(state, "Too many sections");
        return NULL;
    }
    
    Section *sec = &state->sections[state->section_count++];
    strncpy(sec->name, name, sizeof(sec->name) - 1);
    sec->name[sizeof(sec->name) - 1] = '\0';
    sec->address = address;
    sec->attributes = attrs;
    sec->size = 0;
    sec->data = malloc(65536); // 64KB max per section
    if (!sec->data) {
        error_add(state, "Memory allocation failed for section %s", name);
        return NULL;
    }
    memset(sec->data, 0, 65536);
    
    return sec;
}

void section_switch(AssemblerState *state, const char *name) {
    for (int i = 0; i < state->section_count; i++) {
        if (strcmp(state->sections[i].name, name) == 0) {
            state->current_section = i;
            state->pc = state->sections[i].address;
            return;
        }
    }
    
    // Create new section if not found
    section_create(state, name, state->pc, 0x06);
    state->current_section = state->section_count - 1;
}

void error_add(AssemblerState *state, const char *format, ...) {
    if (state->diagnostics.message_count >= 256) return;
    
    va_list args;
    va_start(args, format);
    
    char *msg = state->diagnostics.messages[state->diagnostics.message_count++];
    vsnprintf(msg, 256, format, args);
    
    state->diagnostics.errors++;
    
    va_end(args);
}

void warning_add(AssemblerState *state, const char *format, ...) {
    if (state->diagnostics.message_count >= 256) return;
    
    va_list args;
    va_start(args, format);
    
    char *msg = state->diagnostics.messages[state->diagnostics.message_count++];
    int len = snprintf(msg, 256, "Warning: ");
    vsnprintf(msg + len, 256 - len, format, args);
    
    state->diagnostics.warnings++;
    
    va_end(args);
}

void print_diagnostics(AssemblerState *state) {
    printf("\n=== Assembly Diagnostics ===\n");
    printf("Errors: %d, Warnings: %d\n\n", 
           state->diagnostics.errors, 
           state->diagnostics.warnings);
    
    for (int i = 0; i < state->diagnostics.message_count; i++) {
        printf("%s\n", state->diagnostics.messages[i]);
    }
    
    if (state->diagnostics.errors == 0) {
        printf("\nAssembly successful!\n");
    } else {
        printf("\nAssembly failed with %d error(s)\n", state->diagnostics.errors);
    }
}

// Helper functions

char* string_pool_add(const char *str) {
    if (string_pool_count >= 4096) return NULL;
    
    // Check if string already exists in pool
    for (int i = 0; i < string_pool_count; i++) {
        if (strcmp(string_pool[i], str) == 0) {
            return string_pool[i];
        }
    }
    
    char *copy = strdup(str);
    if (!copy) return NULL;
    
    string_pool[string_pool_count++] = copy;
    return copy;
}

uint32_t hash_string(const char *str) {
    uint32_t hash = 5381;
    int c;
    
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    
    return hash;
}

// Opcode helper functions

OpcodeMetadata* opcode_find(const char *mnemonic) {
    if (!mnemonic) return NULL;
    
    for (int i = 0; instruction_set[i].mnemonic[0]; i++) {
        if (strcasecmp(instruction_set[i].mnemonic, mnemonic) == 0) {
            return &instruction_set[i];
        }
    }
    return NULL;
}

OpcodeMetadata* opcode_by_value(Opcode opcode) {
    for (int i = 0; instruction_set[i].mnemonic[0]; i++) {
        if (instruction_set[i].opcode == opcode) {
            return &instruction_set[i];
        }
    }
    return NULL;
}

int instruction_full_size(Instruction *inst) {
    if (!inst || !inst->info) return 0;
    
    int size = 1; // Opcode
    
    switch (inst->info->opcode) {
        case OP_MOV:
        case OP_MOVW:
        case OP_LOAD:
        case OP_STORE:
        case OP_CMP:
        case OP_TEST:
            // Opcode + Reg + Mode + {Reg(1) or Imm(x)}
            size += 2; // Reg + Mode
            if (inst->operand_count > 1) {
                if (inst->operands[1].mode == AM_IMMEDIATE || inst->operands[1].mode == AM_DIRECT || inst->operands[1].mode == AM_PC_RELATIVE) {
                    size += (inst->info->opcode == OP_MOVW) ? 4 : 2; 
                } else {
                    size += 1; // Register
                }
            }
            break;
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
        case OP_MOD:
        case OP_AND:
        case OP_OR:
        case OP_XOR:
            // Opcode + Reg + Reg + Mode + {Reg(1) or Imm(2)}
            size += 3; // Reg + Reg + Mode
            if (inst->operand_count > 2) {
                if (inst->operands[2].mode == AM_IMMEDIATE || inst->operands[2].mode == AM_DIRECT || inst->operands[2].mode == AM_PC_RELATIVE) {
                    size += 2; // Always 16-bit
                } else {
                    size += 1; // Register
                }
            }
            break;
        case OP_OUT:
        case OP_IN:
            size += 2; // Port/Reg + Reg/Port
            break;
        case OP_INC:
        case OP_DEC:
        case OP_PUSH:
        case OP_POP:
        case OP_NOT:
        case OP_CLR:
            size += 1; // Reg
            break;
        case OP_JMP:
        case OP_JE:
        case OP_JNE:
        case OP_JG:
        case OP_JL:
        case OP_JGE:
        case OP_JLE:
        case OP_CALL:
            size += 2; // 16-bit address
            break;
        case OP_HALT:
        case OP_NOP:
        case OP_RET:
            // Size is 1
            break;
        default:
            // Fallback to basic calculation if unknown
            size = inst->info->size;
            for (int i = 0; i < inst->operand_count; i++) {
                if (inst->operands[i].mode == AM_IMMEDIATE || inst->operands[i].mode == AM_DIRECT) {
                    size += 2;
                }
            }
            break;
    }
    
    return size;
}

bool is_valid_register(int reg_num) {
    return reg_num >= 0 && reg_num < NUM_REGISTERS;
}

bool is_special_register(int reg_num) {
    return reg_num >= REG_PC && reg_num <= REG_LR;
}

const char* register_to_name(int reg_num) {
    static const char* reg_names[] = {
        "R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7",
        "R8", "R9", "R10", "R11", "R12", "R13", "R14", "R15",
        "R16", "R17", "R18", "R19", "R20", "R21", "R22", "R23",
        "R24", "R25", "R26", "R27", "PC", "SP", "FP", "LR"
    };
    
    if (reg_num >= 0 && reg_num < NUM_REGISTERS) {
        return reg_names[reg_num];
    }
    return "INVALID";
}

int name_to_register(const char *name) {
    if (!name) return -1;
    
    // Check for special registers first
    if (strcasecmp(name, "PC") == 0) return REG_PC;
    if (strcasecmp(name, "SP") == 0) return REG_SP;
    if (strcasecmp(name, "FP") == 0) return REG_FP;
    if (strcasecmp(name, "LR") == 0) return REG_LR;
    
    // Check for general registers R0-R31
    if ((name[0] == 'R' || name[0] == 'r') && isdigit(name[1])) {
        char *endptr;
        long reg_num = strtol(name + 1, &endptr, 10);
        if (*endptr == '\0' && reg_num >= 0 && reg_num < NUM_REGISTERS) {
            return (int)reg_num;
        }
    }
    
    return -1; // Invalid register name
}

uint32_t parse_number(const char *str) {
    if (!str) return 0;
    while (*str && isspace(*str)) str++;
    if (!*str) return 0;
    
    // Hex: 0x...
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        return (uint32_t)strtoul(str + 2, NULL, 16);
    }
    // Bin: 0b...
    if (str[0] == '0' && (str[1] == 'b' || str[1] == 'B')) {
        return (uint32_t)strtoul(str + 2, NULL, 2);
    }
    // Decimal
    return (uint32_t)strtoul(str, NULL, 10);
}

void handle_directive(AssemblerState *state, char *line, int pass) {
    if (!state || !line) return;
    
    // Skip optional leading whitespace and dot
    while (*line && isspace(*line)) line++;
    if (*line == '.') line++;
    
    char directive[64];
    int i = 0;
    while (*line && !isspace(*line) && i < 63) {
        directive[i++] = toupper(*line++);
    }
    directive[i] = '\0';
    
    while (*line && isspace(*line)) line++;
    
    if (strcmp(directive, "CODE") == 0 || strcmp(directive, "TEXT") == 0) {
        section_switch(state, ".text");
    } else if (strcmp(directive, "DATA") == 0) {
        section_switch(state, ".data");
    } else if (strcmp(directive, "ORG") == 0) {
        state->pc = parse_number(line);
    } else if (strcmp(directive, "BYTE") == 0) {
        char *token = strtok(line, ",");
        while (token) {
            if (pass == 2) {
                emit_byte(state, (uint8_t)parse_number(token));
            } else {
                state->pc++;
            }
            token = strtok(NULL, ",");
        }
    } else if (strcmp(directive, "WORD") == 0) {
        char *token = strtok(line, ",");
        while (token) {
            if (pass == 2) {
                uint32_t val = parse_number(token);
                emit_byte(state, (uint8_t)(val & 0xFF));
                emit_byte(state, (uint8_t)((val >> 8) & 0xFF));
            } else {
                state->pc += 2;
            }
            token = strtok(NULL, ",");
        }
    } else if (strcmp(directive, "STRING") == 0) {
        char *start = strchr(line, '"');
        if (start) {
            start++;
            char *end = strrchr(start, '"');
            if (end) {
                *end = '\0';
                for (char *p = start; *p; p++) {
                    if (pass == 2) {
                        emit_byte(state, (uint8_t)*p);
                    } else {
                        state->pc++;
                    }
                }
                // Null terminator
                if (pass == 2) emit_byte(state, 0);
                else state->pc++;
            }
        }
    } else if (strcmp(directive, "DWORD") == 0) {
        char *token = strtok(line, ",");
        while (token) {
            if (pass == 2) {
                uint32_t val = parse_number(token);
                emit_byte(state, (uint8_t)(val & 0xFF));
                emit_byte(state, (uint8_t)((val >> 8) & 0xFF));
                emit_byte(state, (uint8_t)((val >> 16) & 0xFF));
                emit_byte(state, (uint8_t)((val >> 24) & 0xFF));
            } else {
                state->pc += 4;
            }
            token = strtok(NULL, ",");
        }
    }
}

void emit_byte(AssemblerState *state, uint8_t byte) {
    if (!state) return;
    
    // Write to memory
    if (state->pc < MEMORY_SIZE) {
        state->memory[state->pc] = byte;
    }
    
    // Update current section size if pc is within it
    Section *sec = &state->sections[state->current_section];
    if (state->pc >= sec->address) {
        uint32_t offset = state->pc - sec->address;
        if (offset + 1 > sec->size) {
            sec->size = offset + 1;
        }
    }
    
    state->pc++;
}

void resolve_references(AssemblerState *state) {
    if (!state) return;
    printf("Resolving references...\n");
}

void handle_include(AssemblerState *state, const char *filename, int pass) {
    if (!state || !filename) return;
    printf("Including file: %s (pass %d)\n", filename, pass);
    FILE *file = fopen(filename, "r");
    if (!file) {
        error_add(state, "Cannot open include file: %s", filename);
        return;
    }
    if (state->include_depth >= MAX_INCLUDE_DEPTH) {
        error_add(state, "Include depth too deep: %s", filename);
        fclose(file);
        return;
    }
    char saved_file[256];
    int saved_line = state->current_line;
    strcpy(saved_file, state->current_file);
    strncpy(state->current_file, filename, sizeof(state->current_file) - 1);
    state->current_file[sizeof(state->current_file) - 1] = '\0';
    state->current_line = 0;
    state->include_stack[state->include_depth++] = file;
    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        state->current_line++;
        
        char original_line[1024];
        strcpy(original_line, line);
        
        Instruction inst;
        memset(&inst, 0, sizeof(inst));
        int result = parse_line(state, line, &inst, pass);
        
        if (result == PARSE_ERROR) {
            error_add(state, "Line %d: Syntax error", state->current_line);
        } else {
             if (pass == 1 && inst.label[0]) {
                 symbol_add(state, inst.label, state->pc, SYM_CODE);
             }
             
             if (result == PARSE_DIRECTIVE) {
                char *dir_start = original_line;
                char *colon = strchr(original_line, ':');
                if (colon) {
                    dir_start = colon + 1;
                }
                handle_directive(state, dir_start, pass);
             } else if (result == PARSE_INSTRUCTION) {
                if (pass == 1) {
                    state->pc += instruction_full_size(&inst);
                } else if (pass == 2) {
                    emit_instruction(state, &inst, pass);
                }
             }
        }
    }
    state->include_depth--;
    fclose(file);
    strcpy(state->current_file, saved_file);
    state->current_line = saved_line;
}

int parse_line(AssemblerState *state, char *line, Instruction *inst, int pass) {
    (void)pass;
    if (!state || !line || !inst) return PARSE_ERROR;
    char *comment = strchr(line, ';');
    if (comment) *comment = '\0';
    while (*line && isspace(*line)) line++;
    if (!*line) return PARSE_EMPTY;
    char *colon = strchr(line, ':');
    if (colon) {
        *colon = '\0';
        char *label = line;
        while (*label && isspace(*label)) label++;
        char *end = label + strlen(label) - 1;
        while (end > label && isspace(*end)) *end-- = '\0';
        if (*label) {
            strncpy(inst->label, label, sizeof(inst->label) - 1);
            inst->label[sizeof(inst->label) - 1] = '\0';
        }
        line = colon + 1;
        while (*line && isspace(*line)) line++;
        if (!*line) return PARSE_INSTRUCTION;
    }
    if (*line == '.') {
        return PARSE_DIRECTIVE;
    }
    char mnemonic[32];
    int i = 0;
    while (*line && !isspace(*line) && i < (int)(sizeof(mnemonic) - 1)) {
        mnemonic[i++] = toupper(*line++);
    }
    mnemonic[i] = '\0';
    inst->info = opcode_find(mnemonic);
    if (!inst->info) {
        error_add(state, "Unknown instruction: %s", mnemonic);
        return PARSE_ERROR;
    }
    while (*line && isspace(*line)) line++;
    inst->operand_count = 0;
    if (*line) {
        char *operand_start = line;
        while (*line && inst->operand_count < MAX_OPERANDS) {
            if (*line == ',') {
                *line = '\0';
                if (!parse_operand(state, operand_start, &inst->operands[inst->operand_count])) return PARSE_ERROR;
                inst->operand_count++;
                operand_start = line + 1;
            }
            line++;
        }
        if (*operand_start) {
            if (!parse_operand(state, operand_start, &inst->operands[inst->operand_count])) return PARSE_ERROR;
            inst->operand_count++;
        }
    }
    return PARSE_INSTRUCTION;
}

bool parse_operand(AssemblerState *state, char *str, Operand *operand) {
    if (!str || !operand) return false;
    while (*str && isspace(*str)) str++;
    char *end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) *end-- = '\0';
    if (!*str) return false;
    int reg = name_to_register(str);
    if (reg >= 0) {
        operand->mode = AM_REGISTER;
        operand->value.reg_num = reg;
        return true;
    }
    if (*str == '#' || isdigit(*str) || *str == '-' || *str == '+') {
        char *num_str = (*str == '#') ? str + 1 : str;
        char *endptr;
        long value = strtol(num_str, &endptr, 0);
        if (*endptr == '\0') {
            operand->mode = AM_IMMEDIATE;
            operand->value.immediate = value;
            return true;
        } else if (*str == '#') {
            operand->mode = AM_IMMEDIATE;
            while (*num_str && isspace(*num_str)) num_str++; // Trim leading space
            strncpy(operand->label, num_str, sizeof(operand->label) - 1);
            operand->label[sizeof(operand->label) - 1] = '\0';
            return true;
        }
    }
    if (*str == '[') {
        char *close = strchr(str, ']');
        if (close) {
            *close = '\0';
            char *addr_str = str + 1;
            while (*addr_str && isspace(*addr_str)) addr_str++;
            char *end_addr = close - 1;
            while (end_addr > addr_str && isspace(*end_addr)) *end_addr-- = '\0';
            
            Symbol *sym = symbol_find(state, addr_str);
            if (sym) {
                operand->mode = AM_DIRECT;
                operand->value.address = sym->value;
                return true;
            }
            int addr_reg = name_to_register(addr_str);
            if (addr_reg >= 0) {
                operand->mode = AM_REGISTER_INDIRECT;
                operand->value.reg_num = addr_reg;
                return true;
            }
            char *endptr;
            long addr = strtol(addr_str, &endptr, 0);
            if (*endptr == '\0') {
                operand->mode = AM_DIRECT;
                operand->value.address = addr;
                return true;
            }
        }
    }
    if (isalpha(*str) || *str == '_' || *str == '.') {
        operand->mode = AM_PC_RELATIVE;
        strncpy(operand->label, str, sizeof(operand->label) - 1);
        operand->label[sizeof(operand->label) - 1] = '\0';
        return true;
    }
    error_add(state, "Invalid operand: %s", str);
    return false;
}

void emit_instruction(AssemblerState *state, Instruction *inst, int pass) {
    if (!state || !inst || !inst->info) return;
    emit_byte(state, inst->info->opcode);
    switch (inst->info->opcode) {
        case OP_MOV:
        case OP_MOVW:
        case OP_LOAD:
        case OP_STORE:
        case OP_CMP:
        case OP_TEST: {
            emit_byte(state, (uint8_t)inst->operands[0].value.reg_num);
            if (inst->operand_count > 1) {
                if (inst->operands[1].mode == AM_IMMEDIATE || inst->operands[1].mode == AM_DIRECT || inst->operands[1].mode == AM_PC_RELATIVE) {
                    emit_byte(state, 1);
                    uint32_t val = inst->operands[1].value.immediate;
                    if (inst->operands[1].label[0]) {
                        Symbol *sym = symbol_find(state, inst->operands[1].label);
                        if (sym) val = sym->value;
                        else if (pass == 2) error_add(state, "Undefined label: %s", inst->operands[1].label);
                    }
                    if (inst->info->opcode == OP_MOVW) {
                        emit_byte(state, (uint8_t)(val & 0xFF));
                        emit_byte(state, (uint8_t)((val >> 8) & 0xFF));
                        emit_byte(state, (uint8_t)((val >> 16) & 0xFF));
                        emit_byte(state, (uint8_t)((val >> 24) & 0xFF));
                    } else {
                        emit_byte(state, (uint8_t)(val & 0xFF));
                        emit_byte(state, (uint8_t)((val >> 8) & 0xFF));
                    }
                } else {
                    emit_byte(state, 0);
                    emit_byte(state, (uint8_t)inst->operands[1].value.reg_num);
                }
            }
            break;
        }
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
        case OP_MOD:
        case OP_AND:
        case OP_OR:
        case OP_XOR: {
            emit_byte(state, (uint8_t)inst->operands[0].value.reg_num);
            emit_byte(state, (uint8_t)inst->operands[1].value.reg_num);
            if (inst->operand_count > 2) {
                if (inst->operands[2].mode == AM_IMMEDIATE || inst->operands[2].mode == AM_DIRECT || inst->operands[2].mode == AM_PC_RELATIVE) {
                    emit_byte(state, 1);
                    uint32_t val = inst->operands[2].value.immediate;
                    if (inst->operands[2].label[0]) {
                        Symbol *sym = symbol_find(state, inst->operands[2].label);
                        if (sym) val = sym->value;
                        else if (pass == 2) error_add(state, "Undefined label: %s", inst->operands[2].label);
                    }
                    emit_byte(state, (uint8_t)(val & 0xFF));
                    emit_byte(state, (uint8_t)((val >> 8) & 0xFF));
                } else {
                    emit_byte(state, 0);
                    emit_byte(state, (uint8_t)inst->operands[2].value.reg_num);
                }
            }
            break;
        }
        case OP_OUT:
        case OP_OUTB: {
            uint32_t port = inst->operands[0].value.immediate;
            emit_byte(state, (uint8_t)(port & 0xFF));
            emit_byte(state, (uint8_t)((port >> 8) & 0xFF));
            emit_byte(state, (uint8_t)inst->operands[1].value.reg_num);
            break;
        }
        case OP_IN:
        case OP_INB: {
            emit_byte(state, (uint8_t)inst->operands[0].value.reg_num);
            uint32_t port = inst->operands[1].value.immediate;
            emit_byte(state, (uint8_t)(port & 0xFF));
            emit_byte(state, (uint8_t)((port >> 8) & 0xFF));
            break;
        }
        case OP_INC:
        case OP_DEC:
        case OP_PUSH:
        case OP_POP:
        case OP_NOT:
        case OP_CLR: {
            emit_byte(state, (uint8_t)inst->operands[0].value.reg_num);
            break;
        }
        case OP_JMP:
        case OP_JE:
        case OP_JNE:
        case OP_JG:
        case OP_JL:
        case OP_JGE:
        case OP_JLE:
        case OP_CALL: {
            uint32_t val = inst->operands[0].value.immediate;
            if (inst->operands[0].label[0]) {
                Symbol *sym = symbol_find(state, inst->operands[0].label);
                if (sym) val = sym->value;
                else if (pass == 2) error_add(state, "Undefined label: %s", inst->operands[0].label);
            }
            emit_byte(state, (uint8_t)(val & 0xFF));
            emit_byte(state, (uint8_t)((val >> 8) & 0xFF));
            break;
        }
        case OP_HALT:
        case OP_NOP:
        case OP_RET:
            break;
        default:
            for (int i = 0; i < inst->operand_count; i++) {
                if (inst->operands[i].mode == AM_REGISTER || inst->operands[i].mode == AM_REGISTER_INDIRECT) {
                    emit_byte(state, (uint8_t)inst->operands[i].value.reg_num);
                } else {
                    uint32_t val = inst->operands[i].value.immediate;
                    emit_byte(state, (uint8_t)(val & 0xFF));
                    emit_byte(state, (uint8_t)((val >> 8) & 0xFF));
                }
            }
            break;
    }
}

void optimize_instructions(AssemblerState *state) {
    if (!state || !state->optimize) return;
    printf("Performing basic optimizations...\n");
    for (int i = 0; i < state->section_count; i++) {
        Section *sec = &state->sections[i];
        if (!sec->data || sec->size == 0) continue;
        for (uint32_t j = 0; j < sec->size - 1; j++) {
            if (sec->data[j] == OP_NOP) {
                for (uint32_t k = j; k < sec->size - 1; k++) sec->data[k] = sec->data[k + 1];
                sec->size--;
                if (j > 0) j--;
            }
        }
    }
}

int write_binary(AssemblerState *state, const char *filename) {
    if (!state || !filename) return 0;
    FILE *f = fopen(filename, "wb");
    if (!f) {
        perror("fopen");
        error_add(state, "Cannot open file %s for writing", filename);
        return 0;
    }
    uint32_t max_addr = 0;
    for (int i = 0; i < state->section_count; i++) {
        if (state->sections[i].address + state->sections[i].size > max_addr) {
            max_addr = state->sections[i].address + state->sections[i].size;
        }
    }
    if (max_addr == 0) max_addr = state->pc;
    
    size_t written = fwrite(state->memory, 1, max_addr, f);
    fclose(f);
    
    if (written != max_addr) {
        return 0;
    }
    return 1;
}
