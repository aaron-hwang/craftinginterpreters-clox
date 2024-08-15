//
// Created by aaron on 8/15/2024.
//

typedef struct {
    // The start of the lexeme we are parsing
    const char* start;
    // The current character we are parsinge
    const char* current;
    int line;
} Scanner;

Scanner scanner;

void initScanner(const char* source) {
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}