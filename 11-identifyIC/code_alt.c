#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 64
#define CHAR_SET 128  // ASCII 문자 집합 크기

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

// 트라이 노드 구조체
typedef struct TrieNode {
    struct TrieNode* children[CHAR_SET]; // 각 ASCII 문자에 대한 자식 노드 포인터 배열
    const char* target;                 // 치환될 문자열
    int is_end;                         // 문자열의 끝인지 여부
} TrieNode;

// 트라이 노드 생성
TrieNode* createNode() {
    TrieNode* node = (TrieNode*)malloc(sizeof(TrieNode));
    node->is_end = 0;                   // 초기 상태는 문자열의 끝 아님
    node->target = NULL;                // 치환 문자열 초기화
    for (int i = 0; i < CHAR_SET; i++) {
        node->children[i] = NULL;       // 모든 자식 노드 NULL로 초기화
    }
    return node;
}

// 하는 일 : 트라이에 문자열 삽입
// 매개변수 : root(트라이 루트), source(변환할 심볼), target(변환될 심볼)
// 작업 방식 : 문자열을 한 글자씩 따라가며 트라이에 노드를 생성하거나 이동한 뒤, 마지막 노드에 target 저장
void insertTrie(TrieNode* root, const char* source, const char* target) {
    TrieNode* current = root;           // 트라이의 루트에서 시작
    while (*source) {                   // source 문자열의 각 문자에 대해 반복
        if (!current->children[(unsigned char)*source]) {
            // 해당 문자가 없는 경우 새로운 노드 생성
            current->children[(unsigned char)*source] = createNode();
        }
        current = current->children[(unsigned char)*source];
        source++;                       // 다음 문자로 이동
    }
    current->is_end = 1;                // 문자열의 끝임을 표시
    current->target = target;           // 치환할 문자열 설정
}

// 하는 일 : 트라이에서 문자열 탐색
// 매개변수 : root(트라이 루트), str(탐색할 문자열 시작점), matched_length(매칭된 문자열 길이 저장할 변수)
// 반환값 : 매칭된 target 문자열, 없으면 NULL
const char* searchTrie(TrieNode* root, const char* str, int* matched_length) {
    TrieNode* current = root;           // 트라이의 루트에서 시작
    *matched_length = 0;                // 초기 매칭 길이 0으로 설정

    while (*str && current->children[(unsigned char)*str]) {
        current = current->children[(unsigned char)*str]; // 다음 문자로 이동
        (*matched_length)++;           // 매칭 길이 증가
        if (current->is_end) {
            // 매칭된 심볼이 존재할 경우 치환할 문자열 반환
            return current->target;
        }
        str++;                         // 입력 문자열의 다음 문자로 이동
    }
    return NULL;                        // 매칭된 심볼 없음
}

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

// 하는 일 : 명령어, 레지스터, 심볼, 의사 명령어를 트라이에 저장
// 매개변수 : input(입력 파일 포인터), root(트라이 루트)
// 작업 방식 : 입력 파일을 한 줄씩 읽고, 문자열을 구분한 뒤 해당 단어를 트라이에 삽입
void read_first_with_trie(FILE* input, TrieNode* root) {
    char line[MAX_LINE_LENGTH];           // 데이터 한 줄을 저장할 버퍼
    const char* delimiter = ", []+-=";    // 구분자 목록
    char* token;                          // 토큰 저장 변수

    // 입력 파일에서 한 줄씩 읽기
    while (fgets(line, MAX_LINE_LENGTH, input) != NULL) {
        if (line[0] == '\n') continue;    // 빈 줄은 건너뜀

        // 한 단어씩 구분하여 읽기
        token = strtok(line, delimiter);
        while (token != NULL) {
            // -----데이터의 모든 단어당 수행하는 동작-----

            // 라벨 / 데이터 심볼 찾기
            if (strchr(token, ':') || strstr(token, "DATA") != NULL) {
                cleanse(token);                   // 필요없는 문자 제거
                insertTrie(root, token, "sym");   // 트라이에 "sym"으로 추가
            }

            cleanse(token);                       // 필요없는 문자 제거

            // 명령어, 레지스터, 의사명령어를 찾아 트라이에 추가
            if (searchTrie(root, token, NULL)) {
                token = strtok(NULL, delimiter);  // 이미 등록된 단어는 넘어감
                continue;
            }
            // 명령어 등록
            for (int i = 0; table_op[i] != NULL; i++) {
                if (strcmp(token, table_op[i]) == 0) {
                    insertTrie(root, token, "op");
                    break;
                }
            }
            // 8비트 레지스터 등록
            for (int i = 0; table_reg8[i] != NULL; i++) {
                if (strcmp(token, table_reg8[i]) == 0) {
                    insertTrie(root, token, "reg8");
                    break;
                }
            }
            // 16비트 레지스터 등록
            for (int i = 0; table_reg16[i] != NULL; i++) {
                if (strcmp(token, table_reg16[i]) == 0) {
                    insertTrie(root, token, "reg16");
                    break;
                }
            }
            // 의사명령어 등록
            for (int i = 0; table_pop[i] != NULL; i++) {
                if (strcmp(token, table_pop[i]) == 0) {
                    insertTrie(root, token, "pop");
                    break;
                }
            }

            // 상수 찾기
            if (isdigit(token[0])) {
                insertTrie(root, token, "num");
            }

            // 다음 단어 탐색
            token = strtok(NULL, delimiter);
        }
    }
}

// 하는 일 : 트라이를 사용하여 텍스트 치환
// 매개변수 : input(입력 파일 포인터), output(출력 파일 포인터), root(트라이 루트)
// 작업 방식 : 입력 파일에서 한 줄씩 읽고, 각 줄에서 트라이를 사용하여 심볼을 치환한 결과를 출력 파일에 저장
void read_second_with_trie(FILE* input, FILE* output, TrieNode* root) {
    char line[MAX_LINE_LENGTH];         // 입력 파일에서 읽은 한 줄 저장할 버퍼

    // 입력 파일에서 한 줄씩 읽음
    while (fgets(line, MAX_LINE_LENGTH, input) != NULL) {
        char result[MAX_LINE_LENGTH] = {0}; // 치환 결과를 저장할 버퍼
        int i = 0, j = 0;                  // i: 입력 문자열의 인덱스, j: 결과 문자열의 인덱스

        // 입력 문자열을 한 글자씩 처리
        while (line[i]) {
            int matched_length = 0;       // 매칭된 문자열 길이 초기화
            const char* target = searchTrie(root, &line[i], &matched_length);

            if (target) {
                // 매칭된 문자열이 있을 경우 치환
                strcat(result, target);   // 치환된 문자열 결과에 추가
                j += strlen(target);      // 결과 문자열 인덱스 갱신
                i += matched_length;      // 입력 문자열 인덱스 갱신
            } else {
                // 매칭되지 않을 경우 원래 문자를 결과에 복사
                result[j++] = line[i++];
            }
        }
        result[j] = '\0';                // 결과 문자열 종료
        fprintf(output, "%s", result);  // 결과 출력
    }
}

// 하는 일 : 트라이 메모리 해제
// 매개변수 : root(트라이 루트)
// 작업 방식 : 재귀적으로 트라이 노드들을 순회하며 메모리 해제
void freeTrie(TrieNode* root) {
    if (!root) return;
    for (int i = 0; i < CHAR_SET; i++) {
        freeTrie(root->children[i]);    // 자식 노드 재귀적으로 해제
    }
    free(root);                         // 현재 노드 해제
}

// 메인 함수
int main() {
    FILE* inputFile = fopen("symbol.txt", "r");  // 입력 파일 열기
    FILE* outputFile = fopen("output.txt", "w"); // 출력 파일 열기
    if (!inputFile || !outputFile) {
        printf("파일을 열 수 없습니다...\n");
        return 1;
    }

    // 트라이 초기화
    TrieNode* root = createNode();

    // 심볼 테이블 초기화
    insertTrie(root, "MOV", "op");      // 명령어 심볼 등록
    insertTrie(root, "AX", "reg16");    // 16비트 레지스터 심볼 등록
    insertTrie(root, "BX", "reg16");
    insertTrie(root, "156", "16");      // 치환할 예제 심볼 등록
    insertTrie(root, "6", "a");

    // 치환 처리
    read_second_with_trie(inputFile, outputFile, root);

    fclose(inputFile);                  // 입력 파일 닫기
    fclose(outputFile);                 // 출력 파일 닫기

    // 트라이 메모리 해제
    freeTrie(root);

    return 0;
}
