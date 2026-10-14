CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I. -Isrc -g
LDFLAGS =

SRC = src/lexer.c src/ast.c src/parser.c src/typechecker.c src/ir.c src/codegen.c src/interpreter.c
OBJ = $(SRC:src/%.c=build/obj/%.o)

TARGET_BIN = build/bin/aeth

.PHONY: all bootstrap test clean test-lexer test-parser test-typechecker test-ir test-codegen test-interpreter test-frontend-backend

all: $(TARGET_BIN)

bootstrap: all

build/obj/%.o: src/%.c | build_dirs
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET_BIN): src/main.c $(OBJ) | build_dirs
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

build_dirs:
	@mkdir -p build/bin build/obj

test: test-lexer test-parser test-typechecker test-ir test-codegen test-interpreter test-frontend-backend

test-lexer: build/obj/lexer.o | build_dirs
	$(CC) $(CFLAGS) tests/test_lexer.c build/obj/lexer.o -o build/bin/test_lexer $(LDFLAGS)
	./build/bin/test_lexer

test-parser: build/obj/lexer.o build/obj/ast.o build/obj/parser.o | build_dirs
	$(CC) $(CFLAGS) tests/test_parser.c build/obj/lexer.o build/obj/ast.o build/obj/parser.o -o build/bin/test_parser $(LDFLAGS)
	./build/bin/test_parser

test-typechecker: build/obj/lexer.o build/obj/ast.o build/obj/parser.o build/obj/typechecker.o | build_dirs
	$(CC) $(CFLAGS) tests/test_typechecker.c build/obj/lexer.o build/obj/ast.o build/obj/parser.o build/obj/typechecker.o -o build/bin/test_typechecker $(LDFLAGS)
	./build/bin/test_typechecker

test-ir: build/obj/lexer.o build/obj/ast.o build/obj/parser.o build/obj/typechecker.o build/obj/ir.o | build_dirs
	$(CC) $(CFLAGS) tests/test_ir.c build/obj/lexer.o build/obj/ast.o build/obj/parser.o build/obj/typechecker.o build/obj/ir.o -o build/bin/test_ir $(LDFLAGS)
	./build/bin/test_ir

test-codegen: build/obj/lexer.o build/obj/ast.o build/obj/parser.o build/obj/typechecker.o build/obj/codegen.o | build_dirs
	$(CC) $(CFLAGS) tests/test_codegen.c build/obj/lexer.o build/obj/ast.o build/obj/parser.o build/obj/typechecker.o build/obj/codegen.o -o build/bin/test_codegen $(LDFLAGS)
	./build/bin/test_codegen

test-interpreter: build/obj/lexer.o build/obj/ast.o build/obj/parser.o build/obj/typechecker.o build/obj/interpreter.o | build_dirs
	$(CC) $(CFLAGS) tests/test_interpreter.c build/obj/lexer.o build/obj/ast.o build/obj/parser.o build/obj/typechecker.o build/obj/interpreter.o -o build/bin/test_interpreter $(LDFLAGS)
	./build/bin/test_interpreter

test-frontend-backend: build/obj/lexer.o build/obj/ast.o build/obj/parser.o build/obj/typechecker.o build/obj/codegen.o build/obj/interpreter.o | build_dirs
	$(CC) $(CFLAGS) tests/test_frontend_backend.c build/obj/lexer.o build/obj/ast.o build/obj/parser.o build/obj/typechecker.o build/obj/codegen.o build/obj/interpreter.o -o build/bin/test_frontend_backend $(LDFLAGS)
	./build/bin/test_frontend_backend

clean:
	rm -rf build
