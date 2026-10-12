# Compiler and flags
CC := gcc
CFLAGS := -Wall -Wextra -I. -Iinclude

# Target executable name
TARGET := lexical_analyzer

# Source files (exclude test input programs like test.c)
SRC := $(filter-out test.c test_%.c, $(wildcard *.c src/*.c))

# Object files generated from sources
OBJ := $(patsubst %.c, %.o, $(SRC))

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJ)
	@$(CC) $(CFLAGS) -o $@ $^

# Rule to compile source files to object files
%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@

# Clean up generated files
clean:
	@rm -f *.exe *.o src/*.o $(TARGET)

.PHONY: all clean
