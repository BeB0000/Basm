# BeboAsm Quick Reference & Comprehensive Guide

## Getting Started

This guide explains how to use the BeboAsm toolchain to develop, simulate, and debug assembly programs.

### 1. Building the Tools
If you haven't built the tools yet, run `make` in the root directory:
```bash
make
```

### 2. Assembling Code
To convert an assembly source file (`.basm`) into a binary (`.bin`), use `beboasm`:
```bash
./beboasm examples/hello.basm hello.bin
```

### 3. Running Code
To run a binary file, use the simulator `bebosim`:
```bash
./bebosim hello.bin
```

### 4. Debugging Code
To step through your code and inspect registers/memory, use `bebodebug`:
```bash
./bebodebug hello.bin
```

---

## 1. Basic Instructions

### Data Transfer
| Instruction | Description | Example |
|------------|-------------|---------|
| `MOV` | Move/Copy | `MOV R1, R2` |
| `LOAD` | Load from memory | `LOAD R1, [R2]` |
| `STORE` | Store to memory | `STORE R1, [R2]` |
| `PUSH` | Push to stack | `PUSH R1` |
| `POP` | Pop from stack | `POP R1` |
| `XCHG` | Exchange | `XCHG R1, R2` |

---

## 2. Arithmetic Operations

| Instruction | Operation | Example | Note |
|------------|-----------|---------|------|
| `ADD` | Addition | `ADD R1, R2, R3` | Modifies flags |
| `SUB` | Subtraction | `SUB R1, R2, R3` | Modifies flags |
| `MUL` | Multiplication | `MUL R1, R2, R3` | May overflow |
| `DIV` | Division | `DIV R1, R2, R3` | Integer division |
| `MOD` | Remainder | `MOD R1, R2, R3` | Division remainder |
| `INC` | Increment | `INC R1` | R1 + 1 |
| `DEC` | Decrement | `DEC R1` | R1 - 1 |

---

## 3. Logical Operations

| Instruction | Operation | Result |
|------------|-----------|--------|
| `AND` | Logical AND | 1 if both = 1 |
| `OR` | Logical OR | 1 if any = 1 |
| `XOR` | Logical XOR | 1 if different |
| `NOT` | Logical NOT | Invert all bits |
| `SHL` | Shift left | Multiply by 2^n |
| `SHR` | Shift right | Divide by 2^n |

---

## 4. Control Flow

### Conditional Jumps
```
CMP src1, src2      ; Compare and set flags

JE label            ; Equal
JNE label           ; Not Equal
JG label            ; Greater
JL label            ; Less
JGE label           ; Greater or Equal
JLE label           ; Less or Equal
```

### Functions
```
CALL label          ; Call function
RET                 ; Return from function
```

---

## 5. Special Registers

| Register | Name | Description |
|----------|------|-------------|
| R0-R27 | General | General purpose |
| R28 | PC | Program counter |
| R29 | SP | Stack pointer |
| R30 | FP | Frame pointer |
| R31 | LR | Link register |

---

## 6. Flags

| Flag | Symbol | Meaning |
|------|--------|---------|
| Zero | Z | Result = 0 |
| Carry | C | Carry occurred |
| Overflow | V | Overflow occurred |
| Negative | N | Result is negative |
| Interrupt | I | Interrupts enabled |
| Direction | D | Operation direction |

---

## 7. Directives

```
.ORG address        ; Set start address
.CODE               ; Code section
.DATA               ; Initialized data
.BSS                ; Uninitialized data
.STACK size         ; Stack size
.EQU name, value    ; Define constant
.BYTE value         ; Single byte (8-bit)
.WORD value         ; Word (16-bit)
.DWORD value        ; Double word (32-bit)
.FLOAT value        ; Float number
.STRING text        ; Text string
.ARRAY size, init   ; Array
.MACRO name, param  ; Macro
```

---

## 8. System Instructions

| Instruction | Description | Example |
|------------|-------------|---------|
| `HALT` | Stop program | `HALT` |
| `NOP` | No operation | `NOP` |
| `IN` | Read from port | `IN R1, #0xC800` |
| `OUT` | Write to port | `OUT #0xC000, R1` |

---

## 9. Common Ports

```
0xC000   = Display/Screen
0xC001   = Printer
0xC800   = Keyboard
0xC801   = Mouse
0x0000   = System ports
```

---

## 10. Quick Examples

### Hello World (Simplified)
This program outputs "Hello" to the standard output port.
```asm
.CODE
START:
    MOV R1, #HELLO_WORLD
PRINT_LOOP:
    LOAD R2, [R1]
    CMP R2, #0
    JE END_PRINT
    OUT #0x01, R2
    INC R1
    JMP PRINT_LOOP
END_PRINT:
    HALT

.DATA
HELLO_MSG:
    .STRING "Hello"
    .BYTE 0
```

### Simple Loop
```asm
MOV R1, #0              ; Counter
loop:
    CMP R1, #10
    JGE end
    INC R1
    JMP loop
end:
    HALT
```

### Function Entry
```asm
.CODE
start:
    MOV R1, #5
    CALL my_func
    HALT

my_func:
    ADD R1, R1, #3
    RET
```

### Using Stack
```asm
PUSH R1             ; Save R1
MOV R1, #100        ; Use R1
POP R1              ; Restore R1
```

### Working with Memory
```asm
MOV R1, #0x2000    ; Address
MOV R2, #42        ; Value
STORE R2, [R1]     ; Write to memory
LOAD R3, [R1]      ; Read from memory
```

---

## 11. Performance Tips

| Comparison | Fastest | Note |
|------------|---------|------|
| Multiply by 8 | `SHL R1, #3` | Faster than `MUL R1, #8` |
| Increment by 1 | `INC R1` | Faster than `ADD R1, #1` |
| Repeated reads | Store in register | Avoid reading memory repeatedly |

---

## 12. Standard Memory Map

```
0x00000000 - 0x0FFFFFFF  : Program code (.CODE)
0x10000000 - 0x1FFFFFFF  : Data (.DATA)
0x20000000 - 0x2FFFFFFF  : Stack (.STACK)
0x30000000 - 0x3FFFFFFF  : Free memory
0xC0000000 - 0xC00FFFFF  : I/O ports
```

---

## 13. Common Error Checklist

- [ ] Do you save registers before using them in functions?
- [ ] Do your PUSH and POP operations match?
- [ ] Do you check array bounds?
- [ ] Do you avoid division by zero?
- [ ] Did you specify stack size?
- [ ] Did you use .ORG to set address?
- [ ] Do you handle overflow conditions?

---

# In-Depth & Comprehensive BeboAsm Guide

## 1. Data Definitions - Complete Reference

### .BYTE - Single Byte (8-bit)
```
.DATA
    small_val: .BYTE 255        ; 0 to 255
    negative: .BYTE -1          ; Stored as 255
    hex_val: .BYTE 0xFF         ; 255 in hex
    bin_val: .BYTE 0b11110000   ; Binary format
```

**Uses**:
- Single characters (ASCII)
- Very small numbers
- Boolean flags
- Status indicators

### .WORD - Word (16-bit)
```
.DATA
    medium_val: .WORD 65535
    color: .WORD 0x00FF00       ; Green color
    address_16: .WORD 0xFFFF    ; 16-bit address
```

**Uses**:
- Medium-sized numbers
- 16-bit addresses
- Color values in some systems
- Array indices

### .DWORD - Double Word (32-bit)
```
.DATA
    large_val: .DWORD 0xFFFFFFFF
    address: .DWORD 0x2000
    timestamp: .DWORD 1234567890
```

**Uses**:
- Memory addresses
- Large integers
- Timestamps
- Enumeration values

### .FLOAT - Floating Point Number
```
.DATA
    pi: .FLOAT 3.14159
    sqrt2: .FLOAT 1.41421
    temp: .FLOAT 37.5
    gravity: .FLOAT 9.81
```

**Uses**:
- Scientific calculations
- Temperature readings
- Percentages
- Ratios and rates

### .STRING - Text String
```
.DATA
    greeting: .STRING "Hello World"
    empty: .STRING ""
    multiline: .STRING "Line 1\nLine 2"
    name: .STRING "Ahmed"
```

**Characteristics**:
- Null-terminated (ends with \0)
- Supports escape sequences (\n, \t, \\)
- Useful for messages and labels
- Perfect for text processing

### .ARRAY - Array Declaration
```
.DATA
    zeros: .ARRAY 10, 0         ; 10 zeros
    ones: .ARRAY 5, 1           ; 5 ones
    scores: .ARRAY 100, 0       ; 100 zeros for scores
    buffer: .ARRAY 256, 0       ; 256-byte buffer
```

**Practical example - Work with arrays**:
```
.DATA
    scores: .ARRAY 5, 0         ; Array of 5 scores
    
.CODE
    ; Initialize first element
    MOV R1, #scores
    MOV R2, #85
    STORE R2, [R1]              ; scores[0] = 85
    
    ; Access second element (each element = 4 bytes)
    ADD R1, R1, #4
    MOV R2, #90
    STORE R2, [R1]              ; scores[1] = 90
    
    ; Access element at index i
    MOV R1, #scores
    MOV R3, #2                  ; Index
    MUL R4, R3, #4              ; Calculate offset
    ADD R1, R1, R4
    LOAD R5, [R1]               ; Load element at index 2
```

---

## 2. Constants and Symbols

### .EQU - Define Constants
```
.EQU MAX_SIZE, 100
.EQU SCREEN_ADDRESS, 0xC000
.EQU KEYBOARD_ADDRESS, 0xC800
.EQU BUFFER_SIZE, 256
.EQU TRUE, 1
.EQU FALSE, 0
```

**Benefits**:
- Easy to modify globally
- Improves code readability
- Avoids magic numbers
- Self-documenting code

**Usage example**:
```
.EQU MAX_ITERATIONS, 1000
.EQU TIMEOUT, 5000

.CODE
start:
    MOV R1, #0
    MOV R2, #MAX_ITERATIONS
    
loop:
    CMP R1, R2
    JGE done
    ; Process
    INC R1
    JMP loop
done:
    HALT
```

### .LABEL - Define Labels
```
.LABEL main_loop
    MOV R1, #10
    CMP R1, #0
    JE end_label
    DEC R1
    JMP main_loop

.LABEL end_label
    HALT
```

### .EXPORT / .IMPORT - Multi-file Programs
```
; file1.basm
.EXPORT add_function
.EXPORT multiply_function

add_function:
    ADD R1, R1, R2
    RET

multiply_function:
    MUL R1, R1, R2
    RET
```

```
; file2.basm
.IMPORT add_function
.IMPORT multiply_function

.CODE
    MOV R1, #5
    MOV R2, #3
    CALL add_function
    HALT
```

---

## 3. Macros and Advanced Directives

### .MACRO - Define Macros
```
.MACRO multiply_by_two, reg
    SHL reg, reg, #1
.ENDMACRO

; Usage:
MOV R1, #5
multiply_by_two R1    ; R1 = 10
```

**Advanced macro with multiple parameters**:
```
.MACRO add_three, dest, src1, src2, src3
    ADD dest, src1, src2
    ADD dest, dest, src3
.ENDMACRO

; Usage:
add_three R4, R1, R2, R3    ; R4 = R1 + R2 + R3
```

**Practical macro example**:
```
.MACRO save_all
    PUSH R1
    PUSH R2
    PUSH R3
    PUSH R4
.ENDMACRO

.MACRO restore_all
    POP R4
    POP R3
    POP R2
    POP R1
.ENDMACRO

.CODE
start:
    save_all
    ; Do work
    restore_all
    HALT
```

### .IFDEF / .ENDIF - Conditional Compilation
```
.IFDEF DEBUG_MODE
    .MACRO print_debug, val
        OUT #0xC000, val
    .ENDMACRO
.ELSE
    .MACRO print_debug, val
        NOP  ; Do nothing in release
    .ENDMACRO
.ENDIF
```

### .INCLUDE - Include Files
```
; library.basm
.MACRO fast_multiply, dest, src1, src2
    MUL dest, src1, src2
.ENDMACRO

.MACRO fast_divide, dest, src1, src2
    DIV dest, src1, src2
.ENDMACRO
```

```
; main.basm
.INCLUDE "library.basm"

.CODE
    MOV R1, #7
    MOV R2, #8
    fast_multiply R3, R1, R2   ; Uses macro from library
    HALT
```

---

## 4. Program Organization

### .ORG - Set Start Address
```
.ORG 0x1000
; Start loading program from address 0x1000
```

**Complete organized program**:
```
.ORG 0x1000

.CODE
start:
    CALL initialize
    CALL main_loop
    HALT

initialize:
    MOV R1, #buffer
    MOV R2, #256
    ; Initialize buffer with zeros
    MOV R3, #0
    MOV R4, #0
init_loop:
    CMP R4, R2
    JGE init_done
    STORE R3, [R1]
    ADD R1, R1, #4
    INC R4
    JMP init_loop
init_done:
    RET

main_loop:
    MOV R1, #0
loop:
    CMP R1, #10
    JE exit
    OUT #0xC000, R1
    INC R1
    JMP loop
exit:
    RET

.DATA
    name: .STRING "BeboAsm Program"
    version: .BYTE 1
    buffer: .ARRAY 256, 0

.BSS
    temp_buffer: .ARRAY 512, 0
    flags: .ARRAY 8, 0

.STACK 0x2000
```

**Memory layout**:
```
0x1000 - 0x1100  : Code section
0x1100 - 0x1200  : Data section
0x1200 - 0x1400  : BSS section
0x2000 - 0x3000  : Stack
```

---

## 5. Registers in Detail

### General Purpose Registers (R0-R27)

**Standard usage convention**:
```
R0-R7    : Temporary values, arithmetic results
R8-R15   : Loop counters, intermediate results
R16-R23  : Function parameters and local variables
R24-R27  : Special purposes (often caller-saved)
```

**Recommended allocation**:
```
R1, R2   : Primary operands
R3       : Result storage
R4       : Loop counter
R5-R7    : Temporary values
R8-R15   : Additional temporaries
R16-R23  : Function parameters (R16=param1, R17=param2, etc.)
```

### Special Registers

**R28 (PC) - Program Counter**:
```
; Points to currently executing instruction
; Auto-increments after each instruction
; Don't modify manually - use CALL/JMP instead

MOV R28, #0x2000   ; WRONG - Don't do this!
```

**R29 (SP) - Stack Pointer**:
```
; Points to top of stack
; Decrements on PUSH (grows downward)
; Increments on POP (shrinks upward)

; Initial: SP = 0x3000
PUSH R1             ; SP becomes 0x2FFC
PUSH R2             ; SP becomes 0x2FF8
POP R2              ; SP becomes 0x2FFC
POP R1              ; SP becomes 0x3000
```

**R30 (FP) - Frame Pointer**:
```
; Marks beginning of current function's local variables
; Useful for accessing stack frame in complex functions

function:
    PUSH R30                ; Save old frame pointer
    MOV R30, R29            ; FP = SP
    SUB R29, R29, #16       ; Allocate 16 bytes for locals
    
    ; Now can access:
    ; [R30-4]   = local var 1
    ; [R30-8]   = local var 2
    ; [R30-12]  = local var 3
    ; [R30-16]  = local var 4
    
    ADD R29, R29, #16       ; Deallocate
    POP R30                 ; Restore old FP
    RET
```

**R31 (LR) - Link Register**:
```
; Automatically saved when CALL executes
; Contains return address
; Automatically used by RET

; Don't modify manually
; Managed automatically by processor
```

---

## 6. Flags Deep Dive

### Z (Zero Flag)
```
Set when: Result of operation = 0
Clear when: Result ¡Ù 0

Usage:
    MOV R1, #0
    CMP R1, #5
    JE is_equal         ; Jumps because Z = 1
    
    MOV R1, #5
    CMP R1, #5
    JE is_equal         ; Jumps because Z = 1
    
    MOV R1, #5
    CMP R1, #3
    JNE not_equal       ; Jumps because Z = 0
```

### C (Carry Flag)
```
Set when: Carry/Borrow from arithmetic operation
Clear otherwise

Arithmetic use:
    MOV R1, #0xFFFFFFFF
    ADD R1, R1, #1
    JC has_carry        ; Jumps (overflow occurred)

Comparison use:
    CMP R1, R2          ; R1 - R2
    JC less_than        ; Jumps if R1 < R2 (borrow)
```

### V (Overflow Flag)
```
Set when: Signed overflow occurs
Clear otherwise

Indicates: Result exceeded signed integer range

Example:
    MOV R1, #0x7FFFFFFF    ; Max positive
    ADD R1, R1, #1
    JV overflow             ; Jumps (overflow)
    
    MOV R1, #0x80000000    ; Min negative
    SUB R1, R1, #1
    JV overflow             ; Jumps (underflow)
```

### N (Negative Flag)
```
Set when: Result is negative (MSB = 1)
Clear when: Result is positive (MSB = 0)

Usage:
    MOV R1, #-5
    CMP R1, #0
    JN is_negative      ; Jumps (N = 1)
    
    MOV R1, #5
    CMP R1, #0
    JN is_negative      ; Doesn't jump (N = 0)
```

### I (Interrupt Flag)
```
Set: Interrupts enabled
Clear: Interrupts disabled

Usage in system programming:
    ; Disable interrupts (atomic operation)
    MOV R1, #0
    MOV R2, I
    AND I, R2, #0
    
    ; Critical section
    
    ; Re-enable interrupts
    OR I, R2, #1
```

---

## 7. Complete Practical Examples

### Example 1: Simple Calculator
```
.ORG 0x1000

.CODE
start:
    MOV R1, #50         ; First operand
    MOV R2, #30         ; Second operand
    MOV R3, #1          ; Operation (1=+, 2=-, 3=*)
    
    CMP R3, #1
    JE do_add
    CMP R3, #2
    JE do_subtract
    CMP R3, #3
    JE do_multiply
    JMP invalid
    
do_add:
    ADD R4, R1, R2
    JMP show_result
do_subtract:
    SUB R4, R1, R2
    JMP show_result
do_multiply:
    MUL R4, R1, R2
    JMP show_result
invalid:
    MOV R4, #0          ; Error code
    
show_result:
    OUT #0xC000, R4     ; Display result
    HALT
```

### Example 2: Bubble Sort Algorithm
```
.ORG 0x1000

.CODE
start:
    MOV R1, #array
    MOV R2, #5          ; Array size
    CALL bubble_sort
    HALT

bubble_sort:
    MOV R3, #0          ; Outer loop
outer_loop:
    CMP R3, R2
    JGE sort_done
    
    MOV R4, #0          ; Inner loop
inner_loop:
    MOV R5, R4
    ADD R5, R5, #1
    CMP R5, R2
    JGE inner_done
    
    ; Load array[R4]
    MOV R6, #array
    MUL R7, R4, #4
    ADD R6, R6, R7
    LOAD R8, [R6]
    
    ; Load array[R4+1]
    MOV R9, #array
    MUL R10, R5, #4
    ADD R9, R9, R10
    LOAD R11, [R9]
    
    ; Compare
    CMP R8, R11
    JLE skip_swap
    
    ; Swap
    STORE R11, [R6]
    STORE R8, [R9]
    
skip_swap:
    INC R4
    JMP inner_loop
    
inner_done:
    INC R3
    JMP outer_loop
    
sort_done:
    RET

.DATA
array: .DWORD 50
       .DWORD 20
       .DWORD 80
       .DWORD 15
       .DWORD 60
```

### Example 3: String Processor
```
.ORG 0x1000

.CODE
start:
    MOV R1, #message
    CALL print_string
    
    MOV R1, #10         ; Newline
    OUT #0xC000, R1
    
    HALT

print_string:
    MOV R2, #0          ; Character counter
loop:
    LOAD R3, [R1]       ; Load character
    CMP R3, #0          ; Null terminator?
    JE done
    
    OUT #0xC000, R3     ; Print character
    
    INC R1              ; Next character
    INC R2              ; Increment counter
    JMP loop
    
done:
    ; R2 contains string length
    RET

.DATA
    message: .STRING "Hello BeboAsm!"
```

### Example 4: Factorial Recursive Function
```
.ORG 0x1000

.CODE
start:
    MOV R1, #5
    CALL factorial
    ; R1 = 120 (5!)
    OUT #0xC000, R1
    HALT

factorial:
    ; R1 = input number
    ; Returns result in R1
    
    CMP R1, #1
    JLE base_case
    
    PUSH R1             ; Save N
    DEC R1              ; N-1
    CALL factorial      ; Recursive call (R1 = (N-1)!)
    POP R2              ; Restore N
    
    MUL R1, R2, R1      ; R1 = N * (N-1)!
    RET
    
base_case:
    MOV R1, #1
    RET
```

### Example 5: Array Sum Calculation
```
.ORG 0x1000

.CODE
start:
    MOV R1, #numbers
    MOV R2, #5          ; Array size
    CALL sum_array
    ; R1 = sum (165)
    OUT #0xC000, R1
    HALT

sum_array:
    ; R1 = array address
    ; R2 = array size
    ; Returns sum in R1
    
    MOV R3, #0          ; Sum = 0
    MOV R4, #0          ; Index = 0
    
loop:
    CMP R4, R2
    JGE done
    
    LOAD R5, [R1]       ; Load array[index]
    ADD R3, R3, R5      ; Sum += array[index]
    
    ADD R1, R1, #4      ; Next element
    INC R4              ; index++
    JMP loop
    
done:
    MOV R1, R3          ; Return sum
    RET

.DATA
    numbers: .DWORD 10
             .DWORD 20
             .DWORD 30
             .DWORD 40
             .DWORD 65
```

---

## 8. Performance Optimization Techniques

### Instruction Selection
```
; Instead of:
MUL R2, R1, #8

; Use:
SHL R2, R1, #3      ; Much faster

; Instead of:
ADD R1, R1, #1

; Use:
INC R1              ; Faster
```

### Register Usage
```
; Bad - Memory access in loop
.DATA
    counter: .DWORD 0

.CODE
loop:
    LOAD R1, [counter]
    INC R1
    STORE R1, [counter]
    CMP R1, #100
    JL loop
```

```
; Good - Use registers
.CODE
start:
    MOV R1, #0          ; Counter in register
loop:
    INC R1
    CMP R1, #100
    JL loop
    ; Use R1 as needed
```

### Loop Unrolling
```
; Slow - 100 iterations
MOV R1, #0
loop:
    ADD R2, R2, #1
    INC R1
    CMP R1, #100
    JL loop
```

```
; Faster - 25 iterations (4x unroll)
MOV R1, #0
loop:
    ADD R2, R2, #1
    ADD R2, R2, #1
    ADD R2, R2, #1
    ADD R2, R2, #1
    INC R1
    CMP R1, #25
    JL loop
```

---

## 9. Debugging Techniques

### Using NOP for Breakpoints
```
.CODE
start:
    MOV R1, #5
    NOP                 ; Breakpoint - debugger can trap here
    ADD R1, R1, #3
    NOP                 ; Another breakpoint
    HALT
```

### Printing Debug Values
```
debug_print:
    ; R1 = value to print
    OUT #0xC000, R1
    
    MOV R2, #10         ; Newline
    OUT #0xC000, R2
    RET

.CODE
start:
    MOV R1, #42
    CALL debug_print    ; Print 42
    HALT
```

### Register Dump Routine
```
dump_registers:
    ; Print R1-R5 for debugging
    MOV R1, #100
    OUT #0xC000, R1
    
    MOV R1, #101
    OUT #0xC000, R1
    
    MOV R1, #102
    OUT #0xC000, R1
    
    MOV R1, #103
    OUT #0xC000, R1
    
    MOV R1, #104
    OUT #0xC000, R1
    RET
```

---

## 10. Common Patterns

### Safe Counter with Bounds Check
```
.CODE
    MOV R1, #0          ; Counter
    MOV R2, #100        ; Maximum
    
loop:
    ; Check bounds
    CMP R1, R2
    JGE counter_full
    
    ; Use counter
    ADD R3, R3, R1
    INC R1
    JMP loop
    
counter_full:
    ; Handle overflow
    HALT
```

### Find Maximum in Array
```
find_max:
    ; R1 = array address
    ; R2 = array size
    ; Returns max in R1
    
    LOAD R3, [R1]       ; R3 = max (start with first)
    MOV R4, #1          ; Index = 1
    
loop:
    CMP R4, R2
    JGE max_done
    
    LOAD R5, [R1]       ; Load current
    CMP R5, R3
    JLE skip
    MOV R3, R5          ; New maximum
skip:
    ADD R1, R1, #4
    INC R4
    JMP loop
    
max_done:
    MOV R1, R3
    RET
```

### Table Lookup
```
.DATA
    days: .STRING "Sun"
          .STRING "Mon"
          .STRING "Tue"
          .STRING "Wed"
          .STRING "Thu"
          .STRING "Fri"
          .STRING "Sat"

.CODE
get_day_name:
    ; R1 = day index (0-6)
    ; Returns address of day name
    
    MUL R2, R1, #3      ; Each entry = 3 chars
    MOV R3, #days
    ADD R3, R3, R2      ; R3 = address of day name
    MOV R1, R3
    RET
```

---

## 11. Best Practices Checklist

### Code Quality
- [ ] Use meaningful register names/aliases
- [ ] Add comments for complex logic
- [ ] Consistent indentation (4 spaces)
- [ ] Group related operations together
- [ ] Use labels for jump targets clearly

### Memory Management
- [ ] Always specify .STACK size
- [ ] Check .ORG address conflicts
- [ ] Verify memory alignment
- [ ] Prevent buffer overflows
- [ ] Initialize critical variables

### Performance
- [ ] Use shifts for power-of-2 multiplication
- [ ] Keep hot loops simple
- [ ] Cache frequently used values
- [ ] Minimize memory accesses
- [ ] Profile before optimizing

### Reliability
- [ ] Validate all inputs
- [ ] Check array bounds
- [ ] Handle edge cases
- [ ] Never divide by zero
- [ ] Verify stack balance (PUSH/POP)

---

## Conclusion

This comprehensive guide covers all aspects of BeboAsm programming from basics to advanced techniques. Key takeaways:

**Remember**:
- **Test thoroughly** - Verify logic correctness before submission
- **Manage stack carefully** - Track PUSH/POP operations
- **Understand flags** - They're the foundation of conditional logic
---

## 12. Troubleshooting

### Common Errors

#### 1. "Unknown instruction (binary data)"
If you see an error like `Unknown instruction (binary data): 01 02 03 04`, it means you are likely trying to run `beboasm` on a binary file (`.bin`) instead of a source file (`.basm`). 
- **Fix**: Use `./bebosim` to run binary files.

#### 2. Label not found
Ensure your labels end with a colon (e.g., `START:`) when defined, but omit the colon when used as an operand (e.g., `JMP START`).

#### 3. Immediate values
Constants must be preceded by `#` (e.g., `MOV R1, #10`).

#### 4. Invalid Operands
Most arithmetic instructions expect registers as operands. If using an immediate value, it usually must be the last operand.
