# =============================================
# S++ Compiler — Makefile
# Tools: Flex + Bison + GCC
# =============================================

CC     = gcc
CFLAGS = -Wall -Iinclude -I.
LFLAGS = -lm
BISON  = bison
FLEX   = flex
TARGET = spp_compiler

# Source files in src/
SRCS = src/ast.c src/symtab.c src/semantic.c src/interpreter.c \
       src/tac.c src/optimizer.c src/codegen.c src/main.c

# Object files
OBJS = $(SRCS:.c=.o) spp_parser.tab.o lex.yy.o

# =============================================
# Build Rules
# =============================================

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LFLAGS)

# Bison: grammar → parser
spp_parser.tab.c spp_parser.tab.h: spp_parser.y
	$(BISON) -d spp_parser.y

# Flex: lexer spec → C source
lex.yy.c: spp_lexer.l spp_parser.tab.h
	$(FLEX) spp_lexer.l

# Compile src/*.c → src/*.o
src/%.o: src/%.c spp_parser.tab.h
	$(CC) $(CFLAGS) -c $< -o $@

# Bison-generated parser
spp_parser.tab.o: spp_parser.tab.c
	$(CC) $(CFLAGS) -c $< -o $@

# Flex-generated lexer
lex.yy.o: lex.yy.c spp_parser.tab.h
	$(CC) $(CFLAGS) -c $< -o $@

# =============================================
# Run & Clean
# =============================================

run: $(TARGET)
	./$(TARGET) input.txt output.txt

clean:
	-del /Q $(TARGET).exe src\*.o *.o spp_parser.tab.c spp_parser.tab.h lex.yy.c 2>nul

.PHONY: all run clean
