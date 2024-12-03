#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 64
#define MAX_SYMBOLS 50

char *table_op[] = { 
    "MOV", "ADD",  "SUB", "INC", "DEC", 
    "CMP", "JMP",  "JE",  "JNE", "JA", 
    "JAE", "JB",   "JBE", "JG",  "JGE", 
    "JL",  "JLE",  "JZ",  "JNC", "CALL", 
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

// 주소 저장 변수
int address = 0;

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

// 하는 일 : 심볼을 추가
void addSymbol(SymbolMappingTable* table, const char* source, const char* target) {
    if (table->symbol_count >= MAX_SYMBOLS) {
        printf("테이블에 남은 공간이 없습니다.");
        return;
    }
    // 중복된 심볼이 있는 지 검사
        for (int i = 0; i < table->symbol_count; i++) {
            if (strcmp(table->mapping_table[i]->source, source) == 0) {
                return; // 중복된 심볼이 있을 때
            }
        }
    
    table->mapping_table[table->symbol_count] = (Symbol*)malloc(sizeof(Symbol));
    table->mapping_table[table->symbol_count]->source = strdup(source);
    table->mapping_table[table->symbol_count]->target = strdup(target);
    table->symbol_count++;
}

// 하는 일 : 리스트에서 특정 심볼을 찾아 테이블에 추가
int processSymbols(char *source, SymbolMappingTable *mappingTable, char* table[], const char *target) {
    int index = 0;

    // 1단계 : list의 요소가 문자열에 있는지 검사
    while (table[index] != NULL) {
        if (strcmp(source, table[index]) == 0) {
            // 3단계 : 심볼 추가
            addSymbol(mappingTable, source, target);
            return 0;
        }
        index++;
    }
    return -1; // 해당 심볼이 없으면 -1 반환
}

// 하는 일 : 명령어, 레지스터, 심볼, 의사 명령어를 찾아 table.mapping_table에 저장.
void read_first(FILE* input, SymbolMappingTable* table) {
    char line[MAX_LINE_LENGTH]; // 데이터 한 줄의 길이
    const char* delimiter = ", []+-=";
    char* token;

    int i = 0, j = 0, wp = 0;
    char op[5][10], *opcode[] = {"mov", ""};

    // 한 줄씩 데이터를 읽음
    while (fgets(line, MAX_LINE_LENGTH, input) != NULL) {
        if (line[0] == '\n') continue;
        //printf("%s\n", line);
        wp = 0; // wp 초기화

        while (line[wp] != '\n') {
            while (line[wp] == ' ' || line[wp] == '\t' || line[wp] == ',') wp++;  // 공백, 탭, 콤마는 통과

            // 공백이나 구분자 아닌 문자를 op 배열에 저장
            while (line[wp] != ' ' && line[wp] != '\t' && line[wp] != '\n' && line[wp] != ',' && line[wp] != '\0') {
                op[j][i] = line[wp];
                i++;
                wp++;
            }
            op[j][i] = '\0';  // 문자열 끝에 null 추가
            i = 0;
            j++;
            if (j >= 5) break;
            address += 2;
        }

        // 데이터 라벨 처리
        if (!stricmp(op[1], "DW")) {
            for(i=0; op[0][i] == '\0'; i++);
            op[0][i-1] = '\0';

            char str[20];
            sprintf(str, "%d", address);
            addSymbol(table, op[0], str);
        }
        j = 0;
    }
}

// 하는 일 : 테이블 출력
void printTable(SymbolMappingTable *table) {
    for (int i = 0; i < table->symbol_count; i++) {
        printf("Source: %s, Target: %s\n", table->mapping_table[i]->source, table->mapping_table[i]->target);
    }
    printf("심볼 개수 : %d", table->symbol_count);
}

// 하는 일 : 테이블을 기반으로 문자열에 있는 심볼 치환
int replace(char* string, SymbolMappingTable* table){
    // 테이블의 모든 심볼에 대해 실행
    for (int i = 0; i < table->symbol_count; i++)
    {
        // 심볼이 문자열에 있는지 확인
        char *pos = strstr(string, table->mapping_table[i]->source);
        if(pos == NULL) continue;
        
        // 기존 단어를 덮어씌울 문자열 temp선언
        char temp[MAX_LINE_LENGTH];
        temp[0] = '\0';

        // symbolpos : 심볼의 시작 인덱스값
        int symbolpos = pos - string;

        // temp 만들기. (앞부분) + 심볼 + (뒷부분)순
        strncpy(temp, string, symbolpos); temp[symbolpos] = '\0';
        strcat(temp, table->mapping_table[i]->target);
        strcat(temp, pos + strlen(table->mapping_table[i]->source));
        
        strcpy(string, temp);
        return 1;
    }
    // 심볼이 없을 때
    return 0;
}

// 하는 일 : 전달받은 문자열에서, 구분자가 있는 인덱스 반환
int str_tok(char* string, const char* delimiter) {
    for(int i = 0; string[i] != '\0'; i++){
        if(string[i] == '\n') {
            return i - 1;
        }

        for(int j = 0; delimiter[j] != '\0'; j++) {
            if(string[i] == delimiter[j]) {
                return i;
            }
        }
    }
    return -1;
}

// 하는 일 : 테이블을 기반으로 치환 후 output에 저장.
void read_second(FILE* input, FILE* output, SymbolMappingTable* table) {
    char line[MAX_LINE_LENGTH]; // 데이터 한 줄의 길이
    const char* delimiter = ", []+-=";
    char token[MAX_LINE_LENGTH];
    int len_token, pos_token;
    

    // 한 줄씩 데이터를 읽음
    while (fgets(line, MAX_LINE_LENGTH, input) != NULL) {
        if (line[0] == '\n') {
            fprintf(output, "\n");
            continue;
        }

        len_token = 0;
        pos_token = 0;
        // 구분자를 만날 때까지 한 글자씩 읽어 token에 저장
        while (line[pos_token] != '\0') {
            len_token = str_tok(line + pos_token, delimiter); // 구분자 찾기

            // 구분자를 찾았으면 token에 저장하고, replace 함수 호출
            if (len_token != -1) {
                // 토큰 추출
                strncpy(token, line + pos_token, len_token);
                token[len_token] = '\0';
                
                replace(token, table);
                //printf("%s", token);
                //printf("%c",line[pos_token + len_token]);

                fprintf(output, "%s", token); // 치환된 token 출력
                fprintf(output, "%c", line[pos_token + len_token]);

                pos_token += len_token + 1; // 구분자 이후로 진행
            } else {
                // 더 이상 구분자가 없다면 나머지 문자열 출력
                fprintf(output, "%s", line + pos_token);
                break;
            }
        }
    }
}


int main() {
    FILE* inputFile  = fopen("test10.asm", "r");
    FILE* stFile     = fopen("st.txt",     "w");
    FILE* outputFile = fopen("output.asm", "w");
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