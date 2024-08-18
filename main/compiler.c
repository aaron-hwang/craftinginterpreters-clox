//
// Created by aaron on 8/15/2024.
//

#include "compiler.h"

#include <stdio.h>
#include <stdlib.h>

#include "scanner.h"
#include "chunk.h";
#include "value.h"

typedef struct {
    Token current;
    Token previous;
    bool hadError;
    // To avoid error cascading
    bool panicMode;
} Parser;

Parser parser;
Chunk* compilingChunk;

static void expression();

static Chunk* currentChunk() {
    return compilingChunk;
}

static void errorAt(Token* token, const char* message) {
    // Ignore errors if we've already seen one
    if (parser.panicMode) return;
    fprintf(stderr, "[line %d] Error", token->line);
    parser.panicMode = true;
    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {

    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }
    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

static void error(const char* message) {
    errorAt(&parser.previous, message);
}

static void errorAtCurrent(const char* message) {
    errorAt(&parser.current, message);
}

static void advance() {
    parser.previous = parser.current;

    while (true) {
        parser.current = scanToken();
        if (parser.current.type != TOKEN_ERROR) break;

        errorAtCurrent(parser.current.start);
    }
}

// om nom nom
static void consume(TokenType type, const char* message) {
    if (parser.current.type == type) {
        advance();
        return;
    }

    errorAtCurrent(message);
}

static void emitByte(uint8_t byte) {
    writeChunk(currentChunk(), byte, parser.previous.line);
}

static void emitBytes(uint8_t b1, uint8_t b2) {
    emitByte(b1);
    emitByte(b2);
}

static void emitReturn() {
    emitByte(OP_RETURN);
}

static void endCompiler() {
    emitReturn();
}

static void grouping() {
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression");
}

static uint8_t makeConstant(Value value) {
    int constant = addConstant(currentChunk(), value);
    if (constant > UINT8_MAX) {
        error("Too many constants in one message");
        return 0;
    }

    return (uint8_t)constant;
}

static void emitConstant(Value value) {
    emitBytes(OP_CONSTANT, makeConstant(value));
}

static void number() {
    double value = strtod(parser.previous.start, NULL);
    emitConstant(value);
}

static void unary() {
    TokenType operatorType = parser.previous.type;

    // Compile operand
    expression();

    // Emit operator exp
    switch (operatorType) {
        case TOKEN_MINUS: emitByte(OP_NEGATE); break;
        // Should be unreachable
        default: return;
    }
}

static void expression() {

}

bool compile(const char* source, Chunk* chunk) {
    initScanner(source);
    compilingChunk = chunk;
    parser.panicMode = false;
    parser.hadError = false;
    advance();
    expression();
    consume(TOKEN_EOF, "Expect end of expression");
    endCompiler();
    return !parser.hadError;
}