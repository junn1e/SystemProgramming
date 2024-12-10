#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>

#define MAX_REG 20
#define MAX_SYM 20

// 하는 일 : 이진수를 정수로 변환
int btoi(char* dig)
{
	register int i = 0, result = 0;
	while (*(dig + i) != '\0')
	{
		if (*(dig + i) == '1')
			result += pow((double)2, (double)strlen(dig + i) - 1);
		i++;
	}
	return result;
}

// 레지스터 테이블 구조체
struct reg
{
	char reg_name[3];
	char reg_num[4];
} Reg[MAX_REG];
// 명령어 테이블 구조체 (datasheet확인)
struct ins
{
	char instruct[6];
	char dest[2];
	char sour[2];
	char word_type[2];
	char ins_code[3];
	char ins_len[2];
	char mod_reg[9];
} Instr[170]; // 각 인스트럭션의 정보를 보관하는 구조체

// 심볼 테이블 구조체
struct symbol_tbl
{
	char symbol[10];
	char word_type[2];
	int lc;
	char data[10];
} Symbol[MAX_SYM];

// 문장 구조체 (add_chk -> pass간 값 임시저장용)
struct sentence
{
	char label[10];
	char _operator[10];
	char operand[3][10];
} Sen;

// 로케이션 카운터 (주소)
int LC, MaxI;

// 하는 일 : 파일에서 레지스터와 명령어 정보를 읽어 테이블에 저장
void Initialize()
{
	int i = 0, j = 1;
	FILE* regi, * inst;
	regi = fopen("reg_tbl.txt", "r");
	inst = fopen("inst_tbl.txt", "r");
	while (!feof(regi))
	{
		fscanf(regi, "%s %*s %s\n", Reg[i].reg_name, Reg[i].reg_num);
		i++;
	} // 레지스터 테이블을 작성한다
	while (!feof(inst))
	{
		fscanf(inst, "%6s%2s%2s%4s%3s%2s%9s\n", Instr[j].instruct,
			Instr[j].dest, Instr[j].sour,
			Instr[j].word_type, Instr[j].ins_code,
			Instr[j].ins_len, Instr[j].mod_reg);
		j++;
	} // 명령어 테이블을 작성한다.
	MaxI = j - 1;
	fclose(regi);
	fclose(inst);
}

// 하는 일 : 문자열의 종류 반환 (0:주소값, 1:워드 reg, 2:바이트 reg, 3:라벨)
int Analyze(char* operand)
{
	char* regist[] = {
	"AX", "AH", "AL",
	"BH", "CH", "DH",
	"BL", "CL", "DL",
	"CX", "DX", "BX",
	"SP", "BP",	"SI", "DI"
	};
	int numReg = 16;

	if ((operand[0] == '-' && isdigit(operand[1])) || isdigit(operand[0]))
		return 0; // immediate 어드레스 모드를 지정
	else {
		for (int i = 0; i < numReg; i++) {
			if (!_stricmp(operand, regist[i])) {
				if (i == 0) return 11; // AX 누산기
				if (i <= 2) return 12; // AL, AH
				return (i < 9) ? 2 : 1; // 바이트 / 워드 레지스터
			}
		}
	}
	return 3; // 라벨
}

// 레지스터 번호 출력
int findRegisterIndex(const char* regName) {
	for (int i = 0; i < MAX_REG; i++) {
		if (!_stricmp(Reg[i].reg_name, regName)) {
			return i;
		}
	}
	return -1;
}
// 심볼 번호 출력
int findSymbolIndex(const char* symbol) {
	for (int i = 0; i < MAX_SYM; i++) {
		if (!strcmp(Symbol[i].symbol, symbol)) {
			return i;
		}
	}
	return -1;
}

#define MAX_INS 31 // 명령어 종류 (Add_Chk에서 심볼 판정 시 사용)
// 오퍼랜드의 어드레스 모드 판정
int Add_Chk(char* sen)
{
	register int k = MaxI;
	int i = 0, j = 0, l = 0, wp = 0;
	char op[5][10] = { 0 },
		// 명령어 집합
		* opcode[] = {
		"MOV", "ADD",  "SUB", "INC", "DEC",
		"CMP", "JMP",  "JE",  "JNE", "JA",
		"JAE", "JB",   "JBE", "JG",  "JGE",
		"JL",  "JLE",  "JZ",  "JNC", "CALL",
		"RET", "PUSH", "POP", "AND", "OR",
		"XOR", "NOT",  "SHL", "SHR", "INT",
		"LOOP",	""
	};

	if (sen[wp] == ';' || sen[wp] == '\n' ||
		sen[wp] == ' ' || sen[wp] == '\0') return -1;

	// 1. 문자열을 구분자를 기준으로 나눠 op[]에 저장
	// [0]: 문자/라벨, [1],[2]: 첫째, 둘째 오퍼랜드
	while (sen[wp] != '\n')
	{
		while (sen[wp] == ' ' || sen[wp] == '\t' || sen[wp] == ',')
			wp++; // 공백, 탭, 콤마는 통과

		while (sen[wp] != ' ' && sen[wp] != '\t' && sen[wp] != '\n' && sen[wp] != ',')
		{
			// 단어 저장
			*(op[j] + i) = sen[wp];
			i++;
			wp++;
		}
		*(op[j] + i) = '\0';
		i = 0;
		j++;
	}

	// op[0]이 명령어일 경우 수행문
	i = 0;
	while (strcmp(opcode[i], ""))
		//  _stricmp : 문자열이 같을 경우 '0' 반환
		if (_stricmp(opcode[i], op[0]))
			i++;
		else
		{
			strcpy(Sen._operator, op[0]);
			for (l = 1; l < j; l++)
				strcpy(Sen.operand[l - 1], op[l]);
			break;
		}
	// op[0]이 명령어가 아닐 경우 수행문
	if (i == MAX_INS)
	{
		strcpy(Sen.label, op[0]);
		strcpy(Sen._operator, op[1]);

		for (l = 2; l < j; l++)
			strcpy(Sen.operand[l - 2], op[l]);
	} // 한 문장을 분석하여 레이블, 오퍼레이터와 오퍼랜드로 분류한다

	// Instr[0]에 데이터 저장
	strcpy(Instr[0].instruct, op[0]);
	switch (Analyze(op[1]))
	{
	case 0:
		strcpy(Instr[0].dest, "i");
		short int data = atoi(op[1]);
		strcpy(Instr[0].word_type, (data >= 0 && data <= 255) ? "b" : "w");
		break;
	case 1:
		strcpy(Instr[0].dest, "r");
		strcpy(Instr[0].word_type, "w");
		break;
	case 2:
		strcpy(Instr[0].dest, "r");
		strcpy(Instr[0].word_type, "b");
		break;
	case 3:
		strcpy(Instr[0].dest, "m");
		break;
	case 11:
		strcpy(Instr[0].dest, "a");
		strcpy(Instr[0].word_type, "w");
	case 12:
		strcpy(Instr[0].dest, "a");
		strcpy(Instr[0].word_type, "b");
	}

	switch (Analyze(op[2]))
	{
	case 0:
		strcpy(Instr[0].sour, "i");
		short int data = atoi(op[2]);
		strcpy(Instr[0].word_type, (data >= 0 && data <= 255) ? "b" : "w");
		break;
	case 1:
		strcpy(Instr[0].sour, "r");
		strcpy(Instr[0].word_type, "w");
		break;
	case 2:
		strcpy(Instr[0].sour, "r");
		strcpy(Instr[0].word_type, "b");
		break;
	case 3:
		strcpy(Instr[0].sour, "m");
		break;
	case 11:
		strcpy(Instr[0].sour, "a");
		strcpy(Instr[0].word_type, "w");
	case 12:
		strcpy(Instr[0].sour, "a");
		strcpy(Instr[0].word_type, "b");
	}

	// Instr[0]과 Instr[] 비교
	while (_stricmp(Instr[k].instruct, Instr[0].instruct) ||
		_stricmp(Instr[k].dest, Instr[0].dest) ||
		_stricmp(Instr[k].sour, Instr[0].sour) ||
		_stricmp(Instr[k].word_type, Instr[0].word_type)) k--;
	//if ( k == 0 ) printf("%s %s %s \n", Instr[0].instruct, Instr[0].dest, Instr[0].sour);
	return k; // k == 0 : symbol
}

// 하는 일 : 첫번째 읽기
void PassI(char* buf)
{
	static int j = 0;
	int i = Add_Chk(buf);
	switch (i)
	{
	case -1: break; // 빈 줄
	case 0: // 지시어 / 심볼
		if (!_stricmp(Sen._operator, "dw"))
			strcpy(Symbol[j].word_type, "w");
		else if (!_stricmp(Sen._operator, "db"))
			strcpy(Symbol[j].word_type, "b");
		strcpy(Symbol[j].symbol, Sen.label);
		strcpy(Symbol[j].data, Sen.operand[0]);
		Symbol[j].lc = LC;
		//printf("%04X: %s", LC, buf);
		if (*Symbol[j].word_type == 'w')
			LC += 2;
		else if (*Symbol[j].word_type == 'b')
			LC += 1;
		j++;
		break;

	default: // 명령어
		//printf("%04X: %s", LC, buf);
		LC += atoi(Instr[i].ins_len);
		break;
	}
}

char* CreateModReg(int i) {
	char modReg[9] = { '\0' };
	strcpy(modReg, Instr[i].mod_reg);
	for (int op = 0; op < 2; op++) {
		const char* operand = Sen.operand[op];
		if (!_stricmp(op == 0 ? Instr[i].dest : Instr[i].sour, "r")) {
			int regIndex = findRegisterIndex(operand);
			char* regPos = strchr(modReg, '?');
			if (regPos == NULL) printf("No space for register assign. code : %s %s %s ",
				Instr[i].instruct, Instr[i].dest, Instr[i].sour);
			else {
				strncpy(regPos, Reg[regIndex].reg_num, 3);
			}
		}
	}
	return modReg;
}

// 하는 일 : 두번째 읽기
void PassII(char* buf)
{
	int i, k = 0;

	i = Add_Chk(buf);
	switch (i)
	{
	case -1:
		break; // 빈 줄
	case 0: // 지시어 / 심볼
	{
		k = findSymbolIndex(Sen.label);

		if (*Symbol[k].word_type == 'w' || *Symbol[k].word_type == 'b') {
			int length = (*Symbol[k].word_type == 'w') ? 2 : 1;

			printf("%04X: %0*s\n", LC, length * 2, Symbol[k].data);
			LC += length;
		}
		else {
			printf("%04X:\n", Symbol[k].lc);
			LC++;
		}
		break;
	}
	default: // 명령어
	{
		//printf("-%-10s", buf);
		//printf("%s %s %s\n", Instr[i].instruct, Instr[i].dest, Instr[i].sour);
		int insLen = atoi(Instr[i].ins_len);

		// data sheet를 참고해 명령어별로 다른 구조의 코드비트를 만듬
		char modReg[5][9] = { '\0' };

		switch (insLen)
		{
		case 1:
		{
			strcpy(modReg[0], "nul");
		}
		case 2:
		{
			// |    mod reg    |
			if (_stricmp(Instr[i].sour, "i")) {
				sprintf(modReg[0], "%02x", btoi(CreateModReg(i)));
			}
			// |      data     |
			else {
				sprintf(modReg[0], "%02x", atoi(Sen.operand[1]));
			} break;
		}
		case 3:
		{
			// |      data     | data if w = 1 |
			if (!_stricmp(Instr[i].sour, "i")) {
				int data = atoi(Sen.operand[1]);
				sprintf(modReg[0], "%02x", data & 0x00FF);
				sprintf(modReg[1], "%02x", (data & 0xFF00) >> 8);
			}
			// |    mod reg    |      data     |
			else {
				sprintf(modReg[0], "%02x", btoi(CreateModReg(i)));
				sprintf(modReg[1], "%02x", atoi(Sen.operand[1]));
			} break;
		}
		case 4:
		{
			// |    mod reg    |      data     | data if w = 1 |
			int data = atoi(Sen.operand[1]);
			sprintf(modReg[0], "%02x", btoi(CreateModReg(i)));
			sprintf(modReg[1], "%02x", data & 0x00FF);
			sprintf(modReg[2], "%02x", (data & 0xFF00) >> 8);
			break;
		}
		case 5:
		{
			// |    mod reg    |      data     | data if w = 1 |      data     |
			int data = atoi(Sen.operand[1]);
			sprintf(modReg[0], "%02x", btoi(CreateModReg(i)));
			sprintf(modReg[1], "%02x", data & 0x00FF);
			sprintf(modReg[2], "%02x", (data & 0xFF00) >> 8);
			sprintf(modReg[1], "%02x", atoi(Sen.operand[2]));
			break;
		}
		case 6:
		{
			// |    mod reg    |      data     | data if w = 1 |      data     | data if w = 1 |
			int data = atoi(Sen.operand[1]);
			sprintf(modReg[0], "%02x", btoi(CreateModReg(i)));
			sprintf(modReg[1], "%02x", data & 0x00FF);
			sprintf(modReg[2], "%02x", (data & 0xFF00) >> 8);
			data = atoi(Sen.operand[2]);
			sprintf(modReg[3], "%02x", data & 0x00FF);
			sprintf(modReg[4], "%02x", (data & 0xFF00) >> 8);
		}
		default:
		{
			break;
		}
		}

		if (!_stricmp(Instr[i].ins_code, "00")) {
			printf("%05x : %x\n", LC++ + 4096, btoi(CreateModReg(i)));
		}
		else {
			printf("%05x : %s\n", LC++ + 4096, Instr[i].ins_code);
		}

		for (int idx = 0; idx < insLen - 1; idx++) {
			printf("%05x : ", LC + 4096);
			printf("%s", modReg[idx]);
			printf("\n");
			LC++;
		}
	}
	}
}

void printSymbolTable() {
	printf("|    Sym     |  wordType  |     lc     |    data    |\n");
	for (int i = 0; i < 20; i++) {
		if (strlen(Symbol[i].symbol) == 0)
			continue;

		printf("| %-10s | %-10s | %-10d | %-10s |\n",
			Symbol[i].symbol,
			Symbol[i].word_type,
			Symbol[i].lc,
			Symbol[i].data);
	}
}

void main()
{
	char buf[50];
	FILE* in;
	in = fopen("test10.asm", "r");
	Initialize();
	//printf("\nPass1:\n");
	while (1)
	{
		fgets(buf, 30, in);
		if (feof(in))
			break;
		PassI(buf);
	}

	//printSymbolTable();

	rewind(in);
	LC = 0;
	//printf("\nPass2:\n");
	while (1)
	{
		fgets(buf, 30, in);
		if (feof(in))
			break;
		PassII(buf);
	}




	fclose(in);

	getchar();
}