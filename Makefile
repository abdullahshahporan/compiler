# =============================================
# S++ Compiler — Makefile
# Tools: Flex + Bison + GCC
# =============================================

CC      = gcc
CFLAGS  = -Wall -Iinclude -Igenerated
LFLAGS  = -lm
BISON   = bison
FLEX    = flex
TARGET  = build/spp_compiler

# Source files in src/
SRCS = src/ast.c src/symtab.c src/semantic.c src/interpreter.c \
       src/tac.c src/optimizer.c src/codegen.c src/main.c

# Object files in build/
OBJS = $(patsubst src/%.c,build/%.o,$(SRCS)) build/spp_parser.tab.o build/lex.yy.o

# =============================================
# Build Rules
# =============================================

all: dirs $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LFLAGS)

# Bison: grammar → generated parser
generated/spp_parser.tab.c generated/spp_parser.tab.h: grammar/spp_parser.y
	$(BISON) -d -o generated/spp_parser.tab.c grammar/spp_parser.y

# Flex: grammar → generated lexer
generated/lex.yy.c: grammar/spp_lexer.l generated/spp_parser.tab.h
	$(FLEX) grammar/spp_lexer.l
	cmd /c move lex.yy.c generated\lex.yy.c

# Compile src/*.c → build/*.o
build/%.o: src/%.c generated/spp_parser.tab.h
	$(CC) $(CFLAGS) -c $< -o $@

# Bison-generated parser
build/spp_parser.tab.o: generated/spp_parser.tab.c
	$(CC) $(CFLAGS) -c $< -o $@

# Flex-generated lexer
build/lex.yy.o: generated/lex.yy.c generated/spp_parser.tab.h
	$(CC) $(CFLAGS) -c $< -o $@

# Create output directories
dirs:
	-@cmd /c "if not exist build mkdir build"
	-@cmd /c "if not exist generated mkdir generated"

# =============================================
# Run & Clean
# =============================================

run: $(TARGET)
	./$(TARGET) tests/input.txt tests/output.txt

clean:
	-cmd /c "del /Q build\*.o build\*.exe 2>nul"
	-cmd /c "del /Q generated\spp_parser.tab.c generated\spp_parser.tab.h generated\lex.yy.c 2>nul"

.PHONY: all run clean dirs
