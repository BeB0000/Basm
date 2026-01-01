#include "../include/beboasm.h"
#include "../include/opcodes.h"
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>

// Forward declarations
void update_flags(SimulatorState *sim, uint32_t result);
int simulator_execute_instruction(SimulatorState *sim);
int execute_mov(SimulatorState *sim);
int execute_add(SimulatorState *sim);
int execute_sub(SimulatorState *sim);
int execute_jmp(SimulatorState *sim);
int execute_je(SimulatorState *sim);
int execute_jne(SimulatorState *sim);
int execute_jg(SimulatorState *sim);
int execute_jl(SimulatorState *sim);
int execute_call(SimulatorState *sim);
int execute_ret(SimulatorState *sim);
uint8_t memory_read_byte(SimulatorState *sim, uint32_t address);
uint16_t memory_read_word(SimulatorState *sim, uint32_t address);
void memory_write_byte(SimulatorState *sim, uint32_t address, uint8_t value);
void memory_write_word(SimulatorState *sim, uint32_t address, uint16_t value);

SimulatorState* simulator_create(AssemblerState *state) {
    SimulatorState *sim = calloc(1, sizeof(SimulatorState));
    if (!sim) return NULL;
    
    // Allocate memory
    sim->memory = calloc(MEMORY_SIZE, 1);
    if (!sim->memory) {
        free(sim);
        return NULL;
    }
    
    // Copy assembled code if provided
    if (state) {
        memcpy(sim->memory, state->memory, MEMORY_SIZE);
    }
    
    // Initialize registers
    for (int i = 0; i < NUM_REGISTERS; i++) {
        sim->registers[i] = 0;
    }
    
    // Set special registers
    sim->pc = 0x0000;          // Start at beginning of code
    sim->sp = 0x3FFF;          // Top of stack (grows downward)
    sim->fp = 0x3FFF;          // Frame pointer starts at stack top
    sim->registers[REG_SP] = sim->sp;
    sim->registers[REG_FP] = sim->fp;
    sim->registers[REG_PC] = sim->pc;
    
    // Initialize statistics
    sim->instructions_executed = 0;
    sim->clock_cycles = 0;
    sim->memory_accesses = 0;
    
    // Initialize pipeline
    sim->pipeline.fetch = 0;
    sim->pipeline.decode = 0;
    sim->pipeline.execute = 0;
    sim->pipeline.writeback = 0;
    sim->pipeline.stalled = false;
    
    // Initialize I/O ports
    memset(sim->io_ports, 0, sizeof(sim->io_ports));
    
    // Initialize interrupt controller
    sim->interrupt.enabled = false;
    sim->interrupt.mask = 0xFF; // All interrupts masked initially
    sim->interrupt.pending = 0;
    
    // Initialize debug state
    sim->single_step = false;
    sim->trace = false;
    sim->trace_file = NULL;
    
    sim->running = true;
    sim->halted = false;
    
    return sim;
}

void simulator_destroy(SimulatorState *sim) {
    if (!sim) return;
    
    if (sim->memory) free(sim->memory);
    if (sim->trace_file) fclose(sim->trace_file);
    free(sim);
}

int simulator_run(SimulatorState *sim) {
    if (!sim) return 0;
    
    printf("Starting simulation...\n");
    printf("PC=0x%04X, SP=0x%04X\n", sim->pc, sim->sp);
    
    clock_t start_time = clock();
    
    while (sim->running) {
        // Check for breakpoints
        for (int i = 0; i < sim->breakpoint_count; i++) {
            if (sim->pc == sim->breakpoints[i]) {
                printf("\n⚡ Breakpoint hit at 0x%04X\n", sim->pc);
                debugger_print_registers(sim);
                return 1;
            }
        }
        
        // Execute one instruction
        if (!simulator_execute_instruction(sim)) {
            printf("\n❌ Execution error at PC=0x%04X\n", sim->pc);
            return 0;
        }
        
        // Update statistics
        sim->instructions_executed++;
        
        // Check for halt
        if (sim->halted) {
            printf("\n⏹️  Processor halted\n");
            break;
        }
        
        // Single step mode
        if (sim->single_step) {
            debugger_print_registers(sim);
            printf("Press Enter to continue, 'q' to quit...\n");
            char c = getchar();
            if (c == 'q' || c == 'Q') break;
        }
        
        // Instruction limit (safety)
        if (sim->instructions_executed > 1000000) {
            printf("\n⚠️  Instruction limit reached\n");
            break;
        }
    }
    
    clock_t end_time = clock();
    double elapsed = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    
    printf("\n=== Simulation Statistics ===\n");
    printf("Instructions executed: %lu\n", (unsigned long)sim->instructions_executed);
    printf("Clock cycles: %lu\n", (unsigned long)sim->clock_cycles);
    printf("Memory accesses: %lu\n", (unsigned long)sim->memory_accesses);
    printf("Execution time: %.3f seconds\n", elapsed);
    printf("IPS: %.0f\n", sim->instructions_executed / elapsed);
    
    return 1;
}

int simulator_step(SimulatorState *sim) {
    if (!sim) return 0;
    
    // Check for breakpoints
    for (int i = 0; i < sim->breakpoint_count; i++) {
        if (sim->pc == sim->breakpoints[i]) {
            printf("\n⚡ Breakpoint hit at 0x%04X\n", sim->pc);
            return 0;
        }
    }
    
    if (!simulator_execute_instruction(sim)) {
        return 0;
    }
    
    sim->instructions_executed++;
    return 1;
}

int simulator_execute_instruction(SimulatorState *sim) {
    // Fetch instruction
    uint8_t opcode = memory_read_byte(sim, sim->pc++);
    
    // Decode and execute
    switch (opcode) {
        case OP_MOV:
            return execute_mov(sim);
        case OP_ADD:
            return execute_add(sim);
        case OP_SUB:
            return execute_sub(sim);
        case OP_JMP:
            return execute_jmp(sim);
        case OP_JE:
            return execute_je(sim);
        case OP_JNE:
            return execute_jne(sim);
        case OP_JG:
            return execute_jg(sim);
        case OP_JL:
            return execute_jl(sim);
        case OP_CALL:
            return execute_call(sim);
        case OP_RET:
            return execute_ret(sim);
        case OP_HALT:
            sim->halted = true;
            return 1;
        case OP_NOP:
            sim->clock_cycles++;
            return 1;
        case OP_OUT: {
            // OUT port, reg (or OUT #port, reg)
            // Implementation expects: OUT port_num, source_val
            // Parser emits: port (imm), reg (reg)
            // Logic: Read port, Read value
            uint8_t port = memory_read_byte(sim, sim->pc++);
            // Skip mode byte for port if present (simplified for now assuming strict format)
            // Actually, assembler emits: port(BYTE), reg(BYTE)
            uint8_t reg = memory_read_byte(sim, sim->pc++);
            uint32_t value = sim->registers[reg];
            printf("OUTPUT [Port 0x%02X]: %d (0x%X) '%c'\n", port, value, value, (char)value);
            sim->clock_cycles += 2;
            return 1;
        }
        case OP_INC: {
            uint8_t reg = memory_read_byte(sim, sim->pc++);
            sim->registers[reg]++;
            update_flags(sim, sim->registers[reg]);
            sim->clock_cycles += 1;
            return 1;
        }
        case OP_DEC: {
            uint8_t reg = memory_read_byte(sim, sim->pc++);
            sim->registers[reg]--;
            update_flags(sim, sim->registers[reg]);
            sim->clock_cycles += 1;
            return 1;
        }
        case OP_LOAD: {
            // LOAD dst, [src]
            uint8_t dst_reg = memory_read_byte(sim, sim->pc++);
            uint8_t mode = memory_read_byte(sim, sim->pc++);
            uint32_t addr;
            
            if (mode == 0) { // Indirect Register
                uint8_t src_reg = memory_read_byte(sim, sim->pc++);
                addr = sim->registers[src_reg];
            } else { // Direct Memory
                 addr = memory_read_word(sim, sim->pc);
                 sim->pc += 2;
            }
            
            sim->registers[dst_reg] = memory_read_byte(sim, addr); // Assuming byte load for now or word? Example uses LOAD R2, [R1]
            // Let's assume word load for versatility or check instruction definition.
            // Example invalidates this: LOAD R2, [R1] (loads 'H') -> Byte load is safer for string
            // But opcode table has LOAD and LOADB. Let's make LOAD generic word/byte?
            // The instruction_set says "LOAD" "Load from memory". Default usually 32-bit/word?
            // But hello.basm loads chars. Strings are bytes.
            // Let's stick to byte for now to make hello world work, or check opcode.
            // Opcode table: LOAD = 0x04.
            // In hello.basm: .STRING "..." -> .BYTE.
            // So LOAD needs to read byte if target is 8-bit? Register is 32-bit.
            // Let's implement as Byte load for "LOAD R2, [R1]" in hello.basm context.
            // Correction: LOAD usually loads machine word. LOADB loads byte.
            // However, the example code uses LOAD. Let's check assembler.c:30: "LOAD", OP_LOAD...
            // It might be intended as byte load for strings.
            // Let's assume byte load to match 'hello.basm' intent of printing chars.
            // Wait, hello.basm uses LOAD R2, [R1]. R1 points to string.
            // If I load word, I get 4 chars.
            // I'll implement as Byte load for now for compatibility with the example.
            
            sim->clock_cycles += 3;
            return 1;
        }
        case OP_CMP: {
            // CMP reg1, reg2/imm
            uint8_t reg1 = memory_read_byte(sim, sim->pc++);
            uint8_t mode = memory_read_byte(sim, sim->pc++);
            uint32_t val1 = sim->registers[reg1];
            uint32_t val2;
            
            if (mode == 0) { // Register
                uint8_t reg2 = memory_read_byte(sim, sim->pc++);
                val2 = sim->registers[reg2];
            } else { // Immediate
                val2 = memory_read_word(sim, sim->pc);
                sim->pc += 2;
            }
            
            uint32_t res = val1 - val2;
            update_flags(sim, res);
            sim->clock_cycles += 2;
            return 1;
        }
        default:
            printf("Unknown opcode: 0x%02X at PC=0x%04X\n", opcode, sim->pc - 1);
            return 0;
    }
    return 1;
}

// MOV instruction: MOV Rdst, Rsrc or MOV Rdst, #imm
int execute_mov(SimulatorState *sim) {
    // Read operands
    uint8_t dest_reg = memory_read_byte(sim, sim->pc++);
    uint8_t mode = memory_read_byte(sim, sim->pc++);
    
    if (mode == 0x00) { // Register mode
        uint8_t src_reg = memory_read_byte(sim, sim->pc++);
        sim->registers[dest_reg] = sim->registers[src_reg];
    } else if (mode == 0x01) { // Immediate mode
        uint16_t imm = memory_read_word(sim, sim->pc);
        sim->pc += 2;
        sim->registers[dest_reg] = imm;
    } else {
        printf("Invalid MOV mode: 0x%02X\n", mode);
        return 0;
    }
    
    sim->clock_cycles += 2;
    return 1;
}

// ADD instruction: ADD Rdst, Rsrc1, Rsrc2 or ADD Rdst, Rsrc1, #imm
int execute_add(SimulatorState *sim) {
    uint8_t dest_reg = memory_read_byte(sim, sim->pc++);
    uint8_t src1_reg = memory_read_byte(sim, sim->pc++);
    uint8_t mode = memory_read_byte(sim, sim->pc++);
    
    uint32_t src1 = sim->registers[src1_reg];
    uint32_t src2;
    
    if (mode == 0x00) { // Register mode
        uint8_t src2_reg = memory_read_byte(sim, sim->pc++);
        src2 = sim->registers[src2_reg];
    } else if (mode == 0x01) { // Immediate mode
        src2 = memory_read_word(sim, sim->pc);
        sim->pc += 2;
    } else {
        printf("Invalid ADD mode: 0x%02X\n", mode);
        return 0;
    }
    
    uint32_t result = src1 + src2;
    
    // Update flags
    update_flags(sim, result);
    
    // Set carry flag if overflow
    if (result > 0xFFFFFFFF) {
        sim->flags |= FLAG_CARRY;
    }
    
    sim->registers[dest_reg] = result & 0xFFFFFFFF;
    sim->clock_cycles += 3;
    
    return 1;
}

// JMP instruction: JMP address
int execute_jmp(SimulatorState *sim) {
    uint16_t target = memory_read_word(sim, sim->pc);
    sim->pc = target;
    sim->clock_cycles += 3;
    return 1;
}

// CALL instruction: CALL address
int execute_call(SimulatorState *sim) {
    uint16_t target = memory_read_word(sim, sim->pc);
    
    // Push return address
    sim->sp -= 2;
    memory_write_word(sim, sim->sp, sim->pc + 2);
    
    // Jump to target
    sim->pc = target;
    sim->clock_cycles += 5;
    
    return 1;
}

// SUB instruction: SUB Rdst, Rsrc1, Rsrc2 or SUB Rdst, Rsrc1, #imm
int execute_sub(SimulatorState *sim) {
    uint8_t dest_reg = memory_read_byte(sim, sim->pc++);
    uint8_t src1_reg = memory_read_byte(sim, sim->pc++);
    uint8_t mode = memory_read_byte(sim, sim->pc++);
    
    uint32_t src1 = sim->registers[src1_reg];
    uint32_t src2;
    
    if (mode == 0x00) { // Register mode
        uint8_t src2_reg = memory_read_byte(sim, sim->pc++);
        src2 = sim->registers[src2_reg];
    } else if (mode == 0x01) { // Immediate mode
        src2 = memory_read_word(sim, sim->pc);
        sim->pc += 2;
    } else {
        printf("Invalid SUB mode: 0x%02X\n", mode);
        return 0;
    }
    
    uint32_t result = src1 - src2;
    update_flags(sim, result);
    
    sim->registers[dest_reg] = result;
    sim->clock_cycles += 3;
    
    return 1;
}

// RET instruction: RET
int execute_ret(SimulatorState *sim) {
    // Pop return address
    uint16_t return_addr = memory_read_word(sim, sim->sp);
    sim->sp += 2;
    
    // Return
    sim->pc = return_addr;
    sim->clock_cycles += 4;
    
    return 1;
}

// Update flags based on result
void update_flags(SimulatorState *sim, uint32_t result) {
    // Zero flag
    if (result == 0) {
        sim->flags |= FLAG_ZERO;
    } else {
        sim->flags &= ~FLAG_ZERO;
    }
    
    // Negative flag (MSB set)
    if (result & 0x80000000) {
        sim->flags |= FLAG_NEGATIVE;
    } else {
        sim->flags &= ~FLAG_NEGATIVE;
    }
}

// Memory access functions
uint8_t memory_read_byte(SimulatorState *sim, uint32_t address) {
    if (address >= MEMORY_SIZE) {
        printf("Memory read out of bounds: 0x%08X\n", address);
        return 0;
    }
    
    // Check watchpoints
    for (int i = 0; i < sim->watchpoint_count; i++) {
        if (sim->watchpoints[i].address == address && 
            (sim->watchpoints[i].watch_type == 'r' || sim->watchpoints[i].watch_type == 'x')) {
            printf("Watchpoint hit: read from 0x%08X\n", address);
        }
    }
    
    sim->memory_accesses++;
    return sim->memory[address];
}

uint16_t memory_read_word(SimulatorState *sim, uint32_t address) {
    if (address >= MEMORY_SIZE - 1) {
        printf("Memory read out of bounds: 0x%08X\n", address);
        return 0;
    }
    
    uint16_t value = sim->memory[address] | (sim->memory[address + 1] << 8);
    sim->memory_accesses += 2;
    return value;
}

void memory_write_byte(SimulatorState *sim, uint32_t address, uint8_t value) {
    if (address >= MEMORY_SIZE) {
        printf("Memory write out of bounds: 0x%08X\n", address);
        return;
    }
    
    // Check watchpoints
    for (int i = 0; i < sim->watchpoint_count; i++) {
        if (sim->watchpoints[i].address == address && 
            sim->watchpoints[i].watch_type == 'w') {
            printf("Watchpoint hit: write to 0x%08X = 0x%02X\n", address, value);
        }
    }
    
    sim->memory[address] = value;
    sim->memory_accesses++;
}

void memory_write_word(SimulatorState *sim, uint32_t address, uint16_t value) {
    if (address >= MEMORY_SIZE - 1) {
        printf("Memory write out of bounds: 0x%08X\n", address);
        return;
    }
    
    sim->memory[address] = value & 0xFF;
    sim->memory[address + 1] = (value >> 8) & 0xFF;
    sim->memory_accesses += 2;
}
// JE instruction: JE address
int execute_je(SimulatorState *sim) {
    uint16_t target = memory_read_word(sim, sim->pc);
    sim->pc += 2; // Jump over address if not taken
    
    if (sim->flags & FLAG_ZERO) {
        sim->pc = target;
        sim->clock_cycles += 1; // Penalty for taken branch
    }
    
    sim->clock_cycles += 2;
    return 1;
}

// JNE instruction: JNE address
int execute_jne(SimulatorState *sim) {
    uint16_t target = memory_read_word(sim, sim->pc);
    sim->pc += 2;
    
    if (!(sim->flags & FLAG_ZERO)) {
        sim->pc = target;
        sim->clock_cycles += 1;
    }
    
    sim->clock_cycles += 2;
    return 1;
}

// JG instruction: JG address
int execute_jg(SimulatorState *sim) {
    uint16_t target = memory_read_word(sim, sim->pc);
    sim->pc += 2;
    
    // Greater (Signed): Z=0 and N=V
    bool zero = (sim->flags & FLAG_ZERO);
    bool neg = (sim->flags & FLAG_NEGATIVE) ? true : false;
    bool ovf = (sim->flags & FLAG_OVERFLOW) ? true : false;
    
    if (!zero && (neg == ovf)) {
        sim->pc = target;
        sim->clock_cycles += 1;
    }
    
    sim->clock_cycles += 2;
    return 1;
}

// JL instruction: JL address
int execute_jl(SimulatorState *sim) {
    uint16_t target = memory_read_word(sim, sim->pc);
    sim->pc += 2;
    
    // Less (Signed): N!=V
    bool neg = (sim->flags & FLAG_NEGATIVE) ? true : false;
    bool ovf = (sim->flags & FLAG_OVERFLOW) ? true : false;
    
    if (neg != ovf) {
        sim->pc = target;
        sim->clock_cycles += 1;
    }
    
    sim->clock_cycles += 2;
    return 1;
}
