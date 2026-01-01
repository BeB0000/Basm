#include "../include/beboasm.h"
#include "../include/opcodes.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    printf("BeboAsm Assembler - Version 1.0\nCreated by Abanoub\n\n");
    
    if (argc < 2) {
        printf("Usage: beboasm <input file> [output file]\n");
        return 1;
    }
    
    const char *input_file = argv[1];
    const char *output_file = (argc >= 3) ? argv[2] : "output.bin";
    
    // Check if input file is binary (based on extension)
    size_t len = strlen(input_file);
    if (len > 4 && strcasecmp(input_file + len - 4, ".bin") == 0) {
        printf("Warning: Input file '%s' appears to be a binary file.\n", input_file);
        printf("If you want to run this program, use the simulator:\n");
        printf("  ./bebosim %s\n\n", input_file);
        printf("Continuing assembly anyway...\n\n");
    }
    
    // Create assembler
    AssemblerState *state = assembler_create(true, false);
    if (!state) {
        fprintf(stderr, "Error: Failed to create assembler state\n");
        return 1;
    }
    
    // Assemble
    if (assemble_file(state, input_file)) {
        // Write output
        if (write_binary(state, output_file)) {
            printf("\nSuccessfully assembled %s to %s\n", input_file, output_file);
        } else {
            fprintf(stderr, "\nFailed to write output file\n");
        }
    } else {
        fprintf(stderr, "\nAssembly failed\n");
    }
    
    assembler_destroy(state);
    return 0;
}