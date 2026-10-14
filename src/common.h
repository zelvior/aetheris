#ifndef AETH_COMMON_H
#define AETH_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <ctype.h>

typedef enum {
    TARGET_NATIVE_64,
    TARGET_NATIVE_32,
    TARGET_WEB,
    TARGET_AI
} TargetArch;

typedef struct {
    TargetArch target;
    bool verbose;
    bool debug;
    const char *output_file;
} CompilerOptions;

typedef struct {
    const char *filename;
    size_t line;
    size_t column;
} SourceLocation;

static inline char* read_file_contents(const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Error: Could not open file '%s'\n", path);
        return NULL;
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char *buffer = (char*)malloc(fileSize + 1);
    if (!buffer) {
        fprintf(stderr, "Error: Not enough memory to read '%s'\n", path);
        fclose(file);
        return NULL;
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize && ferror(file)) {
        fprintf(stderr, "Error: Could not read file '%s'\n", path);
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[bytesRead] = '\0';
    fclose(file);
    return buffer;
}

#endif
