#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 64
#define MAX_SYMBOLS 50

char *table_op[] = { 
    "MOV", "ADD",  "SUB", "INC", "DEC", 
    "CMP", "JMP",  "JE",  "JNE", "CALL", 
    "RET", "PUSH", "POP", "AND", "OR", 
    "XOR", "NOT",  "SHL", "SHR", "INT",
    NULL
};

char *table_reg8[] = { 
    "AL", "AH", "BL", "BH", "CL", 
    "CH", "DL", "DH", NULL
};

char *table_reg16[] = { 
    "AX", "BX", "CX", "DX", "SI", 
    "DI", "BP", "SP", "IP", "FLAGS", 
    "CS", "DS", "SS", "ES", NULL
};

char *table_pop[] = { 
    "DB", "DW", "DD", "DQ", "DT", 
    "EQU", "SEGMENT", "ENDS", "ASSUME", "ORG",
    NULL
};

// 심볼 한 쌍 구조체. source:변환할 심볼, target:변환될 심볼
typedef struct {
    const char* source;
    const char* target;
} Symbol;

// 심볼 테이블 구조체. symbol_count:심볼 개수, mapping_table:Symbol배열
typedef struct {
    int symbol_count;
    Symbol* mapping_table[MAX_SYMBOLS];
} SymbolMappingTable;

// 하는 일 : 문자열에 있는 ':'와 '\n', '\t'제거
void cleanse(char* str) {
    int i, j = 0;
    int length = strlen(str);
    for (i = 0; i < length; i++) {
        if (str[i] != ':' && str[i] != '\n' && str[i] != '\t') {
            str[j++] = str[i];
        }
    }
    str[j] = '\0';
}

// 하는 일 : 리스트에서 특정 심볼을 찾아 테이블에 추가
int processSymbols(char *source, SymbolMappingTable *mappingTable, char* table[], const char *target) {
    int index = 0;

    // 1단계 : list의 요소가 문자열에 있는지 검사
    while (table[index] != NULL) {
        if (strcmp(source, table[index]) == 0) {
            // 2단계 : table에 중복된 심볼이 있는 지 검사
            for (int i = 0; i < mappingTable->symbol_count; i++) {
                if (strcmp(mappingTable->mapping_table[i]->source, source) == 0) {
                    return 0; // 중복된 심볼이 있을 때
                }
            }
            // 3단계 : table에 심볼(source, target) 추가
            if (mappingTable->symbol_count < MAX_SYMBOLS) {
                mappingTable->mapping_table[mappingTable->symbol_count] = (Symbol*)malloc(sizeof(Symbol));
                mappingTable->mapping_table[mappingTable->symbol_count]->source = strdup(source);
                mappingTable->mapping_table[mappingTable->symbol_count]->target = strdup(target);
                mappingTable->symbol_count++;
                return 1;
            }
            return -1; // 테이블이 전부 찼을 때
        }
        index++;
    }
    return -1; // 해당 심볼이 없으면 -1 반환
}

// 하는 일 : 명령어, 레지스터, 심볼, 의사 명령어를 찾아 table.mapping_table에 저장.
void read_first(FILE* input, SymbolMappingTable* table) {
    char line[MAX_LINE_LENGTH]; // 데이터 한 줄의 길이
    const char* delimiter = ", ";
    char* token;

    // 한 줄씩 데이터를 읽음.
    while (fgets(line, MAX_LINE_LENGTH, input) != NULL) {
        if (line[0] == '\n') continue;

        // 한 단어씩 데이터를 읽음
        token = strtok(line, delimiter);
        while (token != NULL)
        {
            // -----데이터의 모든 단어당 수행하는 동작-----

            // 라벨 / 데이터 심볼 찾기
            if (strchr(token, ':') || strcmp(token, "DATA") == 0) {
                cleanse(token);
                table->mapping_table[table->symbol_count] = (Symbol*)malloc(sizeof(Symbol));
                table->mapping_table[table->symbol_count]->source = strdup(token);
                table->mapping_table[table->symbol_count]->target = strdup("sym");
                table->symbol_count++;
            }

            cleanse(token);
            // 명령어, 레지스터, 의사명령어 찾기
            if (processSymbols(token, table, table_op, "op") == 1) break;
            if (processSymbols(token, table, table_reg8, "reg8") == 1) break;
            if (processSymbols(token, table, table_reg16, "reg16") == 1) break;
            if (processSymbols(token, table, table_pop, "pop") == 1) break;
            
            // 상수 찾기
            if (isdigit(token[0])) {
                table->mapping_table[table->symbol_count] = (Symbol*)malloc(sizeof(Symbol));
                table->mapping_table[table->symbol_count]->source = strdup(token);
                table->mapping_table[table->symbol_count]->target = strdup("num");
                table->symbol_count++;
            }

            // 다음 단어 검색
            token = strtok(NULL, delimiter);
        }
    }
}

// 하는 일 : 테이블 출력
void printTable(SymbolMappingTable *table) {
    for (int i = 0; i < table->symbol_count; i++) {
        printf("Source: %s, Target: %s\n", table->mapping_table[i]->source, table->mapping_table[i]->target);
    }
}

// 하는 일 : 테이블을 기반으로 치환 후 output에 저장.
void read_second(FILE* input, FILE* output, SymbolMappingTable* table) {
    char line[MAX_LINE_LENGTH]; // 입력 파일에서 읽은 한 줄을 저장할 버퍼

    // 입력 파일에서 한 줄씩 읽음
    while (fgets(line, MAX_LINE_LENGTH, input) != NULL)
    {
        // 테이블의 각 심볼마다 한 번씩 실행
        for (int i = 0; i < table->symbol_count; i++)
        {
            // 테이블에 있는 심볼(source)이 문자열에 있는지 확인
            char *pos = strstr(line, table->mapping_table[i]->source);
            while (pos != NULL)
            {
                // 기존 줄을 덮어씌울 문자열 temp선언
                char temp[MAX_LINE_LENGTH];
                temp[0] = '\0';

                // symbolpos : 심볼의 시작 인덱스값
                int symbolpos = pos - line;

                // temp 만들기. (앞부분) + 심볼 + (뒷부분)순
                strncpy(temp, line, symbolpos);
                temp[symbolpos] = '\0';
                strcat(temp, table->mapping_table[i]->target);
                strcat(temp, pos + strlen(table->mapping_table[i]->source));

                // 기존 줄 덮어씌우기
                strcpy(line, temp);

                // 방금 바꾼 심볼이 같은 줄에 또 있는지 확인
                pos = strstr(line, table->mapping_table[i]->source);
            }
        }
        // 결과 output에 출력
        fprintf(output, "%s", line);
    }
}


int main() {
    FILE* inputFile  = fopen("symbol.txt", "r");
    FILE* stFile     = fopen("st.txt",     "w");
    FILE* outputFile = fopen("output.txt", "w");
    if (!inputFile || !stFile || !outputFile) {
        printf("파일을 열 수 없습니다...\n");
        return 1;
    }

    SymbolMappingTable mapping_table;
    mapping_table.symbol_count = 0;

    read_first(inputFile, &mapping_table);
    printTable(&mapping_table);

    // 테이블 내용을 st.txt파일에 저장
    for (int i = 0; i < mapping_table.symbol_count; i++) {
        fprintf(stFile, "%s %s\n", 
        mapping_table.mapping_table[i]->source,
        mapping_table.mapping_table[i]->target);
    }

    rewind(inputFile);

    read_second(inputFile, outputFile, &mapping_table);

    fclose(inputFile);
    fclose(stFile);
    fclose(outputFile);
    return 0;
}