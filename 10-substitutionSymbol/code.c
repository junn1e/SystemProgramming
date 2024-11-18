#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 64
#define MAX_SYMBOLS 20

typedef struct {
    char symbol[MAX_LINE_LENGTH];
    int address;
} Symbol;

void cleanse(char* str) {
    int i, j = 0;
    int length = strlen(str);
    for (i = 0; i < length; i++) {
        if (str[i] != ':' && str[i] != '\n') {
            str[j++] = str[i];
        }
    }
    str[j] = '\0';
}

int addSymbol(Symbol* symbolTable, int* symbolCount, char* symbol, int address) {
    for (int i = 0; i < *symbolCount; i++) {
        if (strcmp(symbolTable[i].symbol, symbol) == 0) {
            return 0;
        }
    }

    if (*symbolCount < MAX_SYMBOLS) {
        strcpy(symbolTable[*symbolCount].symbol, symbol);
        symbolTable[*symbolCount].address = address;
        (*symbolCount)++;
        return 1;
    }

    return -1;
}

void read_first(FILE* input, Symbol* symbolTable, int* symbolCount) {
    char line[MAX_LINE_LENGTH];
    const char* delimiter = ", ";
    int address = 0;
    char* token;
    while (fgets(line, MAX_LINE_LENGTH, input) != NULL) {
        if (line[0] == '\n') continue;
        token = strtok(line, delimiter);
        while (token != NULL)
        {
            if (strchr(token, ':') || strstr(token, "DATA")) { //�ɺ��� ã���� ��:
                cleanse(token);
                addSymbol(symbolTable, symbolCount, token, address);
            }
            token = strtok(NULL, delimiter);
            address += 1;
        }
    }
}

void read_second(FILE* input, Symbol* symbolTable, int symbolCount, FILE* output) {
    char line[MAX_LINE_LENGTH];

    while (fgets(line, MAX_LINE_LENGTH, input) != NULL) {
        for (int i = 0; i < symbolCount; i++) {
            char* pos = strstr(line, symbolTable[i].symbol);
            while (pos != NULL) {
                char symbolAddr[MAX_LINE_LENGTH];
                sprintf(symbolAddr, "%d", symbolTable[i].address);
                int symbolpos = pos - line;

                char temp[MAX_LINE_LENGTH];
                temp[0] = '\0';

                strncpy(temp, line, symbolpos); //ġȯ �˰�����
                temp[symbolpos] = '\0';
                strcat(temp, symbolAddr);
                strcat(temp, pos + strlen(symbolTable[i].symbol));

                strcpy(line, temp);
                pos = strstr(line, symbolTable[i].symbol);
            }
        }
        fprintf(output, "%s", line);
    }
}

int main() {
    FILE* inputFile  = fopen("symbol.txt", "r");
    FILE* stFile     = fopen("st.txt",     "w");
    FILE* outputFile = fopen("output.txt", "w");
    if (!inputFile || !stFile || !outputFile) {
        printf("파일을 열 수 없습니다.\n");
        return 1;
    }
    Symbol symbolTable[MAX_SYMBOLS];
    int symbolCount = 0;
    read_first(inputFile, symbolTable, &symbolCount);
    for (int i = 0; i < symbolCount; i++) {
        fprintf(stFile, "%s %d\n", symbolTable[i].symbol, symbolTable[i].address);
    }

    rewind(inputFile);
    read_second(inputFile, symbolTable, symbolCount, outputFile);

    fclose(inputFile);
    fclose(stFile);
    fclose(outputFile);
    return 0;
}