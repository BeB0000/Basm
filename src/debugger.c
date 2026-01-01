#include "../include/opcodes.h"

// Forward declarations for local functions
void process_command(SimulatorState *sim, const char *cmd);
void print_help(void);

void debugger_start(SimulatorState *sim) {
    printf("BeboAsm Debugger v1.0\n");
    printf("Type 'help' for commands\n\n");
    
    sim->single_step = true;
    
    char line_buf[256];
    
    while (1) {
        printf("(bebodebug) ");
        fflush(stdout);
        
        if (!fgets(line_buf, sizeof(line_buf), stdin)) break;
        
        // Remove newline
        line_buf[strcspn(line_buf, "\n")] = 0;
        
        if (strlen(line_buf) > 0) {
            process_command(sim, line_buf);
        }
    }
}


void process_command(SimulatorState *sim, const char *cmd) {
    char command[64];
    char args[256];
    
    if (sscanf(cmd, "%63s %255[^\n]", command, args) < 1) {
        return;
    }
    
    if (strcmp(command, "help") == 0 || strcmp(command, "?") == 0) {
        print_help();
    } else if (strcmp(command, "run") == 0 || strcmp(command, "r") == 0) {
        sim->single_step = false;
        simulator_run(sim);
        sim->single_step = true;
    } else if (strcmp(command, "step") == 0 || strcmp(command, "s") == 0) {
        simulator_step(sim);
        debugger_print_registers(sim);
        debugger_disassemble(sim, sim->pc, 1);
    } else if (strcmp(command, "break") == 0 || strcmp(command, "b") == 0) {
        uint32_t addr;
        if (sscanf(args, "0x%x", &addr) == 1 || sscanf(args, "%u", &addr) == 1) {
            debugger_add_breakpoint(sim, addr);
        }
    } else if (strcmp(command, "registers") == 0 || strcmp(command, "reg") == 0) {
        debugger_print_registers(sim);
    } else if (strcmp(command, "memory") == 0 || strcmp(command, "mem") == 0) {
        uint32_t addr, size = 16;
        if (sscanf(args, "0x%x %u", &addr, &size) >= 1 ||
            sscanf(args, "%u %u", &addr, &size) >= 1) {
            debugger_print_memory(sim, addr, size);
        }
    } else if (strcmp(command, "disassemble") == 0 || strcmp(command, "dis") == 0) {
        uint32_t addr = sim->pc;
        uint32_t count = 5;
        if (sscanf(args, "0x%x %u", &addr, &count) >= 1 ||
            sscanf(args, "%u %u", &addr, &count) >= 1) {
            debugger_disassemble(sim, addr, count);
        } else {
            debugger_disassemble(sim, sim->pc, 5);
        }
    } else if (strcmp(command, "quit") == 0 || strcmp(command, "q") == 0) {
        exit(0);
    } else {
        printf("Unknown command: %s\n", command);
    }
}

void debugger_print_registers(SimulatorState *sim) {
    printf("\n=== Registers ===\n");
    
    // General purpose registers
    for (int i = 0; i < 16; i++) {
        printf("R%02d: 0x%08X  ", i, sim->registers[i]);
        if ((i + 1) % 4 == 0) printf("\n");
    }
    
    // Special registers
    printf("\nPC: 0x%08X  SP: 0x%08X  FP: 0x%08X\n", 
           sim->pc, sim->sp, sim->fp);
    
    // Flags
    printf("Flags: [%c%c%c%c%c%c%c%c]\n",
           (sim->flags & FLAG_ZERO) ? 'Z' : '-',
           (sim->flags & FLAG_CARRY) ? 'C' : '-',
           (sim->flags & FLAG_OVERFLOW) ? 'V' : '-',
           (sim->flags & FLAG_NEGATIVE) ? 'N' : '-',
           (sim->flags & FLAG_INTERRUPT) ? 'I' : '-',
           (sim->flags & FLAG_DECIMAL) ? 'D' : '-',
           (sim->flags & FLAG_BREAK) ? 'B' : '-',
           (sim->flags & FLAG_DEBUG) ? 'D' : '-');
    
    printf("Instructions: %lu  Cycles: %lu\n", 
           (unsigned long)sim->instructions_executed, (unsigned long)sim->clock_cycles);
}

void debugger_print_memory(SimulatorState *sim, uint32_t address, uint32_t size) {
    printf("\nMemory at 0x%08X:\n", address);
    
    for (uint32_t i = 0; i < size; i += 16) {
        printf("0x%04X: ", address + i);
        
        // Hex dump
        for (int j = 0; j < 16 && (i + j) < size; j++) {
            if (j == 8) printf(" ");
            printf("%02X ", memory_read_byte(sim, address + i + j));
        }
        
        // ASCII dump
        printf(" |");
        for (int j = 0; j < 16 && (i + j) < size; j++) {
            uint8_t c = memory_read_byte(sim, address + i + j);
            printf("%c", (c >= 32 && c < 127) ? c : '.');
        }
        printf("|\n");
    }
}

void debugger_disassemble(SimulatorState *sim, uint32_t address, uint32_t count) {
    printf("\nDisassembly:\n");
    
    uint32_t pc = address;
    for (uint32_t i = 0; i < count; i++) {
        printf("%c 0x%04X: ", (pc == sim->pc) ? '>' : ' ', pc);
        
        uint8_t opcode = memory_read_byte(sim, pc);
        pc++;
        
        // Simple disassembly (expand this for full instruction set)
        switch (opcode) {
            case OP_MOV: {
                uint8_t dst = memory_read_byte(sim, pc++);
                uint8_t mode = memory_read_byte(sim, pc++);
                printf("MOV R%d, ", dst);
                if (mode == 0) printf("R%d", memory_read_byte(sim, pc++));
                else { printf("0x%04X", memory_read_word(sim, pc)); pc += 2; }
                break;
            }
            case OP_ADD:
            case OP_SUB: {
                uint8_t dst = memory_read_byte(sim, pc++);
                uint8_t src1 = memory_read_byte(sim, pc++);
                uint8_t mode = memory_read_byte(sim, pc++);
                printf("%s R%d, R%d, ", (opcode == OP_ADD) ? "ADD" : "SUB", dst, src1);
                if (mode == 0) printf("R%d", memory_read_byte(sim, pc++));
                else { printf("0x%04X", memory_read_word(sim, pc)); pc += 2; }
                break;
            }
            case OP_LOAD: {
                uint8_t dst = memory_read_byte(sim, pc++);
                uint8_t mode = memory_read_byte(sim, pc++);
                printf("LOAD R%d, ", dst);
                if (mode == 0) printf("[R%d]", memory_read_byte(sim, pc++));
                else { printf("[0x%04X]", memory_read_word(sim, pc)); pc += 2; }
                break;
            }
            case OP_CMP: {
                uint8_t src1 = memory_read_byte(sim, pc++);
                uint8_t mode = memory_read_byte(sim, pc++);
                printf("CMP R%d, ", src1);
                if (mode == 0) printf("R%d", memory_read_byte(sim, pc++));
                else { printf("0x%04X", memory_read_word(sim, pc)); pc += 2; }
                break;
            }
            case OP_OUT: {
                uint8_t port = memory_read_byte(sim, pc++);
                uint8_t reg = memory_read_byte(sim, pc++);
                printf("OUT #0x%02X, R%d", port, reg);
                break;
            }
            case OP_INC:
            case OP_DEC:
                printf("%s R%d", (opcode == OP_INC) ? "INC" : "DEC", memory_read_byte(sim, pc++));
                break;
            case OP_JMP:
            case OP_JE:
            case OP_JNE:
            case OP_JG:
            case OP_JL: {
                const char *mnem[] = {"JMP", "JE", "JNE", "JG", "JL"};
                const uint8_t ops[] = {OP_JMP, OP_JE, OP_JNE, OP_JG, OP_JL};
                const char *m = "J??";
                for(int k=0; k<5; k++) if(ops[k] == opcode) m = mnem[k];
                printf("%s 0x%04X", m, memory_read_word(sim, pc));
                pc += 2;
                break;
            }
            case OP_HALT:
                printf("HALT");
                break;
            case OP_NOP:
                printf("NOP");
                break;
            default:
                printf("DB 0x%02X", opcode);
        }
        printf("\n");
    }
}

void debugger_add_breakpoint(SimulatorState *sim, uint32_t address) {
    if (sim->breakpoint_count >= 256) {
        printf("Breakpoint table full\n");
        return;
    }
    
    sim->breakpoints[sim->breakpoint_count++] = address;
    printf("Breakpoint set at 0x%08X\n", address);
}

void print_help(void) {
    printf("\nAvailable commands:\n");
    printf("  run/r           - Run program\n");
    printf("  step/s          - Execute single instruction\n");
    printf("  break/b ADDR    - Set breakpoint\n");
    printf("  registers/reg   - Show registers\n");
    printf("  memory/mem ADDR [SIZE] - Show memory\n");
    printf("  disassemble/dis [ADDR] [COUNT] - Disassemble code\n");
    printf("  quit/q          - Exit debugger\n");
    printf("  help/?          - This help\n");
}