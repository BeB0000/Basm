#include "../include/beboasm.h"
#include "../include/opcodes.h"

void optimize_instructions(AssemblerState *state) {
    if (!state || state->optimization_level == 0) return;
    
    printf("Optimization level: %d\n", state->optimization_level);
    
    // Apply optimizations based on level
    if (state->optimization_level >= 1) {
        peephole_optimize(state);
        constant_folding(state);
    }
    
    if (state->optimization_level >= 2) {
        dead_code_elimination(state);
        strength_reduction(state);
    }
    
    if (state->optimization_level >= 3) {
        loop_optimization(state);
        register_allocation(state);
    }
}

// Peephole optimization: look for small patterns
void peephole_optimize(AssemblerState *state) {
    int optimizations = 0;
    
    for (int i = 0; i < state->instruction_count - 1; i++) {
        Instruction *inst1 = &state->instructions[i];
        Instruction *inst2 = &state->instructions[i + 1];
        
        // Pattern: MOV R1, R2 followed by MOV R2, R1
        if (inst1->opcode == OP_MOV && inst2->opcode == OP_MOV &&
            inst1->operands[0].type == OP_REGISTER &&
            inst1->operands[1].type == OP_REGISTER &&
            inst2->operands[0].type == OP_REGISTER &&
            inst2->operands[1].type == OP_REGISTER &&
            inst1->operands[0].value == inst2->operands[1].value &&
            inst1->operands[1].value == inst2->operands[0].value) {
            
            // Remove both (they cancel each other)
            remove_instructions(state, i, 2);
            optimizations++;
            i--; // Check again from this position
        }
        
        // Pattern: ADD R1, R1, #0
        else if (inst1->opcode == OP_ADD &&
                 inst1->operands[0].type == OP_REGISTER &&
                 inst1->operands[1].type == OP_REGISTER &&
                 inst1->operands[2].type == OP_IMMEDIATE &&
                 inst1->operands[0].value == inst1->operands[1].value &&
                 inst1->operands[2].value == 0) {
            
            // Replace with NOP
            inst1->opcode = OP_NOP;
            inst1->num_operands = 0;
            optimizations++;
        }
    }
    
    if (optimizations > 0) {
        printf("Peephole optimizations: %d\n", optimizations);
    }
}

// Constant folding: evaluate expressions at compile time
void constant_folding(AssemblerState *state) {
    int optimizations = 0;
    
    for (int i = 0; i < state->instruction_count; i++) {
        Instruction *inst = &state->instructions[i];
        
        if (inst->opcode == OP_ADD &&
            inst->operands[1].type == OP_IMMEDIATE &&
            inst->operands[2].type == OP_IMMEDIATE) {
            
            // Compute constant value
            int32_t result = inst->operands[1].value + inst->operands[2].value;
            
            // Replace with MOV with immediate
            inst->opcode = OP_MOV;
            inst->operands[1].type = OP_IMMEDIATE;
            inst->operands[1].value = result;
            inst->num_operands = 2;
            
            optimizations++;
        }
    }
    
    if (optimizations > 0) {
        printf("Constant folding: %d\n", optimizations);
    }
}

// Dead code elimination: remove unused code
void dead_code_elimination(AssemblerState *state) {
    int removed = 0;
    
    // Mark all labels that are referenced
    bool label_used[MAX_SYMBOLS] = {0};
    
    // First pass: find used labels
    for (int i = 0; i < state->instruction_count; i++) {
        Instruction *inst = &state->instructions[i];
        
        for (int j = 0; j < inst->num_operands; j++) {
            if (inst->operands[j].type == OP_LABEL) {
                Symbol *sym = symbol_find(state, inst->operands[j].label);
                if (sym) {
                    label_used[sym - state->symbols] = true;
                }
            }
        }
    }
    
    // Second pass: remove unreachable code after unconditional jumps
    for (int i = 0; i < state->instruction_count - 1; i++) {
        Instruction *inst = &state->instructions[i];
        
        if (inst->opcode == OP_JMP || inst->opcode == OP_RET || 
            inst->opcode == OP_HALT) {
            
            // Remove instructions after unconditional jump/return/halt
            // (unless they have a label that's used elsewhere)
            int j = i + 1;
            while (j < state->instruction_count) {
                if (state->instructions[j].label[0] != '\0') {
                    Symbol *sym = symbol_find(state, state->instructions[j].label);
                    if (sym && label_used[sym - state->symbols]) {
                        break; // This label is used, stop removal
                    }
                }
                j++;
            }
            
            if (j > i + 1) {
                remove_instructions(state, i + 1, j - (i + 1));
                removed += j - (i + 1);
            }
        }
    }
    
    if (removed > 0) {
        printf("Dead code elimination: %d instructions removed\n", removed);
    }
}