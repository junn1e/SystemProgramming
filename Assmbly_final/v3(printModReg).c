#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>

#define MAX_REG 20
#define MAX_SYM 20

// �ϴ� �� : �������� ������ ��ȯ
int btoi(char* dig)
{
	register int i = 0, ret = 0;
	while (*(dig + i) != '\0')
	{
		if (*(dig + i) == '1')
			ret += pow((double)2, (double)strlen(dig + i) - 1);
		i++;
	}
	return ret;
}

// �������� ���̺� ����ü
struct reg
{
	char reg_name[3];
	char reg_num[4];
} Reg[MAX_REG];
// ��ɾ� ���̺� ����ü (datasheetȮ��)
struct ins
{
	char instruct[6];
	char dest[2];
	char sour[2];
	char word_type[2];
	char ins_code[3];
	char ins_len[2];
	char mod_reg[9];
} Instr[170]; // �� �ν�Ʈ������ ������ �����ϴ� ����ü

// �ɺ� ���̺� ����ü
struct symbol_tbl
{
	char symbol[10];
	char word_type[2];
	int lc;
	char data[10];
} Symbol[MAX_SYM];

// ���� ����ü (add_chk -> pass�� �� �ӽ������)
struct sentence
{
	char label[10];
	char _operator[10];
	char operand[3][10];
} Sen;

// �����̼� ī���� (�ּ�)
int LC, MaxI;

// �ϴ� �� : ���Ͽ��� �������Ϳ� ��ɾ� ������ �о� ���̺� ����
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
	} // �������� ���̺��� �ۼ��Ѵ�
	while (!feof(inst))
	{
		fscanf(inst, "%6s%2s%2s%4s%3s%2s%9s\n", Instr[j].instruct,
			Instr[j].dest, Instr[j].sour,
			Instr[j].word_type, Instr[j].ins_code,
			Instr[j].ins_len, Instr[j].mod_reg);
		j++;
	} // ��ɾ� ���̺��� �ۼ��Ѵ�.
	MaxI = j - 1;
	fclose(regi);
	fclose(inst);
}

// �ϴ� �� : ���ڿ��� ���� ��ȯ (0:�ּҰ�, 1:2����Ʈ reg, 2:1����Ʈ reg, 3:��)
int Analyze(char* operand)
{
	char* regist[] = {
	"AL", "AX", "CL", "CX", "DL", "DX",
	"BL", "BX", "AH", "SP", "CH", "BP",
	"DH", "SI", "BH", "DI"
	};
	int numReg = 16;

	if ((operand[0] == '-' && isdigit(operand[1])) || isdigit(operand[0]))
		return 0; // immediate ��巹�� ��带 ����
	else {
		for (int i = 0; i < numReg; i++) {
			if (!strcmp(operand, regist[i])) {
				if (i < 4)
					return 1;  // 2����Ʈ ��������
				else
					return 2;  // 1����Ʈ ��������
			}
		}
	}
	return 3; // ��
}

// �������� ��ȣ ���
int findRegisterIndex(const char* regName) {
	for (int i = 0; i < MAX_REG; i++) {
		if (!_stricmp(Reg[i].reg_name, regName)) {
			return i;
		}
	}
	return -1;
}
// �ɺ� ��ȣ ���
int findSymbolIndex(const char* symbol) {
	for (int i = 0; i < MAX_SYM; i++) {
		if (!strcmp(Symbol[i].symbol, symbol)) {
			return i;
		}
	}
	return -1;
}

#define MAX_INS 31 // ��ɾ� ���� (Add_Chk���� �ɺ� ���� �� ���)
// ���۷����� ��巹�� ��� ����
int Add_Chk(char* sen)
{
	register int k = MaxI;
	int i = 0, j = 0, l = 0, wp = 0;
	char op[5][10] = { 0 },
		// ��ɾ� ����
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

	// 1. ���ڿ��� �����ڸ� �������� ���� op[]�� ����
	// [0]: ����/��, [1],[2]: ù°, ��° ���۷���
	while (sen[wp] != '\n')
	{
		while (sen[wp] == ' ' || sen[wp] == '\t' || sen[wp] == ',')
			wp++; // ����, ��, �޸��� ���

		while (sen[wp] != ' ' && sen[wp] != '\t' && sen[wp] != '\n' && sen[wp] != ',')
		{
			// �ܾ� ����
			*(op[j] + i) = sen[wp];
			i++;
			wp++;
		}
		*(op[j] + i) = '\0';
		i = 0;
		j++;
	}

	// op[0]�� ��ɾ��� ��� ���๮
	i = 0;
	while (strcmp(opcode[i], ""))
		//  _stricmp : ���ڿ��� ���� ��� '0' ��ȯ
		if (_stricmp(opcode[i], op[0]))
			i++;
		else
		{
			strcpy(Sen._operator, op[0]);
			for (l = 1; l < j; l++)
				strcpy(Sen.operand[l - 1], op[l]);
			break;
		}
	// op[0]�� ��ɾ �ƴ� ��� ���๮
	if (i == MAX_INS)
	{
		strcpy(Sen.label, op[0]);
		strcpy(Sen._operator, op[1]);

		for (l = 2; l < j; l++)
			strcpy(Sen.operand[l - 2], op[l]);
	} // �� ������ �м��Ͽ� ���̺�, ���۷����Ϳ� ���۷���� �з��Ѵ�

	// Instr[0]�� ������ ����
	strcpy(Instr[0].instruct, op[0]);
	// ù ��° ���۷��� �м�
	switch (Analyze(op[1]))
	{
	case 0:
		strcpy(Instr[0].dest, "i"); 			// ��ð�
		break;
	case 1:
		strcpy(Instr[0].dest, "r");				// ��������
		strcpy(Instr[0].word_type, "w");	// ���� Ÿ��
		break;
	case 2:
		strcpy(Instr[0].dest, "r");				// ��������
		strcpy(Instr[0].word_type, "b");	// ����Ʈ Ÿ��
		break;
	case 3:
		strcpy(Instr[0].dest, "m"); 			// �޸�
		break;
	}

	// �� ��° ���۷��� �м�
	switch (Analyze(op[2]))
	{
	case 0:
		strcpy(Instr[0].sour, "i"); 			// ��ð�
		break;
	case 1:
		strcpy(Instr[0].sour, "r");				// ��������
		strcpy(Instr[0].word_type, "w");	// ���� Ÿ��
		break;
	case 2:
		strcpy(Instr[0].sour, "r");				// ��������
		strcpy(Instr[0].word_type, "b");	// ����Ʈ Ÿ��
		break;
	case 3:
		strcpy(Instr[0].sour, "m");				// �޸�
		break;
	}

	// Instr[0]�� Instr[] ��
	//printf("%s %s %s : ", Instr[0].instruct, Instr[0].dest, Instr[0].sour);
	while (_stricmp(Instr[k].instruct, Instr[0].instruct) ||
		_stricmp(Instr[k].dest, Instr[0].dest) ||
		_stricmp(Instr[k].sour, Instr[0].sour)) k--;
	// ���� �� while�� ���� �������� ���̱� || strcmp(Instr[k].word_type, Instr[0].word_type)
	//if ( k == 0 ) printf("%s %s %s \n", Instr[0].instruct, Instr[0].dest, Instr[0].sour);
	return k; // k == 0 : symbol
}

// �ϴ� �� : ù��° �б�
void PassI(char* buf)
{
	static int j = 0;
	int i = Add_Chk(buf);
	switch (i)
	{
	case -1: // �� ��
		break;

	case 0:  //���þ� / �ɺ�
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

	default: // ��ɾ�
		//printf("%04X: %s", LC, buf);
		LC += atoi(Instr[i].ins_len);
		break;
	}
}

// �ϴ� �� : �ι�° �б�
void PassII(char* buf)
{
	int i, k = 0;

	i = Add_Chk(buf);
	switch (i)
	{
	case -1: // �� ��
		break;

	case 0:  // ���þ� / �ɺ�
		k = findSymbolIndex(Sen.label);

		//  *Symbol[k].word_type == 'w' �� !strcmp(Symbol[k].word_type, "w") �� ������
		if (*Symbol[k].word_type == 'w' || *Symbol[k].word_type == 'b') {
			int length = (*Symbol[k].word_type == 'w') ? 2 : 1;

			//printf("%04X: %0*X %20s", LC, length * 2, atoi(Symbol[k].data), buf);
			LC += length;
		}
		break;

	default: // ��ɾ�
		printf("\n%04X %3s : ", LC, Instr[i].ins_code);

		// ��ɾ� ��Ʈ �κ��� ������ ����
		char tempModReg[9] = { '?' };
		strcpy(tempModReg, Instr[i].mod_reg);
		tempModReg[8] = '\0';

		// �������� �ڵ� �ٿ��ֱ�
		for (int op = 0; op < 2; op++) {
			const char* operand = Sen.operand[op];
			if (!_stricmp(op == 0 ? Instr[i].dest : Instr[i].sour, "r")) {
				int regIndex = findRegisterIndex(operand);

				// '?'�κп� �������� �ڵ� �ٿ��ֱ�
				char* regPos = strchr(tempModReg, '?');
				if (regPos == NULL) printf("No space for register assign. code : %s %s %s ",
					Instr[i].instruct, Instr[i].dest, Instr[i].sour);
				else {
					strncpy(regPos, Reg[regIndex].reg_num, 3);
				}
			}
		}

		// r/i/a to r/a �϶�
		if (_stricmp(Instr[i].dest, "m") && _stricmp(Instr[i].sour, "m")) {
		}

		// �޸� ����
		int symbolIndex = -1;
		if (!strcmp(Instr[i].dest, "m")) {
			symbolIndex = findSymbolIndex(Sen.operand[0]);
		}
		else if (!strcmp(Instr[i].sour, "m")) {
			symbolIndex = findSymbolIndex(Sen.operand[1]);
		}

		printf("%s", tempModReg);

		LC += atoi(Instr[i].ins_len);
		break;
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
	printf("\nPass1:\n");
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
	printf("\nPass2:\n");

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