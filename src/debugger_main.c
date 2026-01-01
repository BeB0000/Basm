#include "../include/beboasm.h"
#include "../include/opcodes.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    printf("BeboAsm Debugger - Version 1.0\nCreated by Abanoub\n\n");
    
    if (argc < 2) {
        printf("Usage: bebodebug <binary file>\n");
        return 1;
    }
    
    const char *filename = argv[1];
    
    // Create simulator state (debugger uses simulator)
    SimulatorState *sim = simulator_create(NULL);
    if (!sim) {
        fprintf(stderr, "Error: Failed to create simulator\n");
        return 1;
    }
    
    // Load binary file
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        simulator_destroy(sim);
        return 1;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);
    
    if (file_size > MEMORY_SIZE) {
        fprintf(stderr, "Error: File too large for memory (%ld bytes > %d bytes)\n", file_size, MEMORY_SIZE);
        fclose(file);
        simulator_destroy(sim);
        return 1;
    }
    
    // Read file into memory
    size_t read_size = fread(sim->memory, 1, file_size, file);
    fclose(file);
    
    if (read_size != (size_t)file_size) {
        fprintf(stderr, "Error: Failed to read file\n");
        simulator_destroy(sim);
        return 1;
    }
    
    printf("Loaded %ld bytes from %s\n", read_size, filename);
    
    // Initialize running state
    sim->running = true;
    
    // Start debugger
    debugger_start(sim);
    
    // Clean up
    simulator_destroy(sim);
    
    return 0;
}
