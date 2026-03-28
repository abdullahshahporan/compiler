# =============================================
# S++ Compiler - Build Script (Makefile)
# Tools: Flex (lexer) + Bison (parser) + GCC
# =============================================

CC = gcc
CFLAGS = -Wall -lm
BISON = bison
FLEX = flex

TARGET = spp_compiler

all: $(TARGET)

$(TARGET): spp_parser.tab.c lex.yy.c
	$(CC) -o $(TARGET) spp_parser.tab.c lex.yy.c $(CFLAGS)

spp_parser.tab.c spp_parser.tab.h: spp_parser.y
	$(BISON) -d spp_parser.y

lex.yy.c: spp_lexer.l spp_parser.tab.h
	$(FLEX) spp_lexer.l

run: $(TARGET)
	./$(TARGET) input.txt output.txt

clean:
	rm -f $(TARGET) spp_parser.tab.c spp_parser.tab.h lex.yy.c spp_parser.output

.PHONY: all run clean
