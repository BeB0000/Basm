# BeboAsm Makefile
CC = gcc
CFLAGS = -Wall -Wextra -O2 -g -I./include
LDFLAGS = -lm
TARGET = beboasm
SIM_TARGET = bebosim
DEBUG_TARGET = bebodebug


# Common sources
LIB_SRC = src/assembler.c
LIB_OBJ = $(LIB_SRC:.c=.o)

# Simulator sources
SIM_LIB_SRC = src/simulator.c src/debugger.c
SIM_LIB_OBJ = $(SIM_LIB_SRC:.c=.o)

# Main Object files (not linked together)
ASM_MAIN_OBJ = src/main.o
SIM_MAIN_OBJ = src/simulator_main.o
DBG_MAIN_OBJ = src/debugger_main.o

# Default target
all: $(TARGET) $(SIM_TARGET) $(DEBUG_TARGET)

# Main assembler
$(TARGET): $(ASM_MAIN_OBJ) $(LIB_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Simulator
$(SIM_TARGET): $(SIM_MAIN_OBJ) $(SIM_LIB_OBJ) $(LIB_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Debugger
$(DEBUG_TARGET): $(DBG_MAIN_OBJ) $(SIM_LIB_OBJ) $(LIB_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Object files compile rule
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -f src/*.o *.bin *.lst *.o $(TARGET) $(SIM_TARGET) $(DEBUG_TARGET)

# Install
install: all
	cp $(TARGET) $(SIM_TARGET) $(DEBUG_TARGET) /usr/local/bin/
	chmod +x /usr/local/bin/$(TARGET) /usr/local/bin/$(SIM_TARGET) /usr/local/bin/$(DEBUG_TARGET)

# Uninstall
uninstall:
	rm -f /usr/local/bin/$(TARGET) /usr/local/bin/$(SIM_TARGET) /usr/local/bin/$(DEBUG_TARGET)

# Run tests
test: all
	@echo "Running tests..."
	@./run_tests.sh

# Format code
format:
	clang-format -i src/*.c include/*.h

# Dependencies
depend:
	$(CC) -MM $(SRC) $(SIM_SRC) $(OPT_SRC) > .depend

-include .depend

# Documentation
doc:
	doxygen Doxyfile

.PHONY: all clean install uninstall test format doc