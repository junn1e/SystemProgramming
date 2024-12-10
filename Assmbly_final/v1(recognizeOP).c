#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>

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
} Reg[20];
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
} Instr[100]; // �� �ν�Ʈ������ ������ �����ϴ� ����ü

// �ɺ� ���̺� ����ü
struct symbol_tbl
{
	char symbol[10];
	char word_type[2];
	int lc;
	char data[10];
} Symbol[20];

// ���� ����ü (add_chk -> pass�� �� �ӽ������)
struct sentence
{
	char label[10];
	char _operator[10];
	char operand[3][10];
} Sen;

// �ִ� ��ɾ�, �ɺ�, �� ����
int MaxI, MaxS, LC;

// �ϴ� �� : ���Ͽ��� �������Ϳ� ��ɾ� ������ �о� ���̺� ����
void Initialize()
{
	int i = 0, j = 1;
	FILE* regi, * inst;
	regi = fopen("reg_tbl.txt", "r");
	inst = fopen("inst_tbl.txt", "r");
	while (!feof(regi))
	{
		fscanf(regi, "%s%s\n", Reg[i].reg_name, Reg[i].reg_num);
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
	int i = 0;
	char* regist[] = { "AX", "BX", "CX", "DX", "AL", "BL", "CL", "DL", "AH", "BH", "CH", "DH", 0x00 }; // ���������� �̸��� ����

	if (isdigit(operand[0]))
		return 0; // immediate ��巹�� ��带 ����
	else
		while (regist[i] != 0x00)
			if (!strcmp(operand, regist[i]))
			{
				if (i < 4)
					return 1;
				else
					return 2;
			}
			else
				i++;
	return 3;
}

#define MAX_INS 31			 // ��ɾ��� �ִ� ������ ����
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
	printf("%-10s : ", op[0]);

	// op[0]�� ��ɾ��� ��� ���๮
	i = 0;
	while (strcmp(opcode[i], ""))
		//  _stricmp : ���ڿ��� ���� ��� '0' ��ȯ
		if (_stricmp(opcode[i], op[0]))
			i++;
		else
		{
			printf("op\n");
			strcpy(Sen._operator, op[0]);
			for (l = 1; l < j; l++)
				strcpy(Sen.operand[l - 1], op[l]);
			break;
		}
	// op[0]�� ��ɾ �ƴ� ��� ���๮
	if (i == MAX_INS)
	{
		printf("sym\n");
		strcpy(Sen.label, op[0]);
		strcpy(Sen._operator, op[1]);

		for (l = 2; l < j; l++)
			strcpy(Sen.operand[l - 2], op[1]);
	} // �� ������ �м��Ͽ� ���̺�, ���۷����Ϳ� ���۷���� �з��Ѵ�

	// OP�ڵ� ����
	strcpy(Instr[0].instruct, op[0]); // bug????

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
	//printf("%s %s %s %s %s : ", Instr[0].instruct, Instr[0].dest, Instr[0].sour, Instr[0].word_type);
	//int k = MaxI;
	while (_stricmp(Instr[k].instruct, Instr[0].instruct) || strcmp(Instr[k].dest, Instr[0].dest) || strcmp(Instr[k].sour, Instr[0].sour) || strcmp(Instr[k].word_type, Instr[0].word_type))
		k--;
	return k; // k == 0 : symbol
}

// �ϴ� �� : ù��° �б�
void PassI(char* buf)
{
	int i;
	static int j = 0;
	i = Add_Chk(buf);

	if (i)
	{
		//printf("%04X: %s", LC, buf);
		LC += atoi(Instr[i].ins_len);
	}
	else
	{
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
	}
}

void PassII(char* buf)
{
	int i, j = 0, k = 0;

	i = Add_Chk(buf);

	if (i)
	{
		printf("%04x: %3s", LC, Instr[i].ins_code);
		if (!strcmp(Instr[i].dest, "r"))
		{
			while (_stricmp(Reg[j].reg_name, Sen.operand[0]))
				j++;
			strncpy(strchr(Instr[0].mod_reg, '?'), Reg[j].reg_num, 3);
		}
		j = 0;
		if (!strcmp(Instr[i].sour, "r"))
		{
			while (_stricmp(Reg[j].reg_name, Sen.operand[1]))
				j++;
			strncpy(strchr(Instr[0].mod_reg, '?'), Reg[j].reg_num, 3);
		}
		if (strcmp(Instr[i].dest, "m") && strcmp(Instr[i].sour, "m"))
			printf("%02X %s", btoi(Instr[i].mod_reg), buf);
		else
		{
			if (!strcmp(Instr[i].dest, "m"))
				while (strcmp(Symbol[k].symbol, Sen.operand[0]))
					k++;

			else if (!strcmp(Instr[i].sour, "m"))
				while (strcmp(Symbol[k].symbol, Sen.operand[1]))
					k++;
			printf(" %02X %04X %s", btoi(Instr[i].mod_reg), Symbol[k].lc, buf);
		}
		LC += atoi(Instr[i].ins_len);
	}

	else
	{
		k = 0;
		while (strcmp(Symbol[k].symbol, Sen.label))
			k++;
		if (!strcmp(Symbol[k].word_type, "w"))
			printf("%04X:%04X %20s", LC, atoi(Symbol[k].data), buf);
		if (!strcmp(Symbol[k].word_type, "b"))
			printf("%04X: %02X %20s", LC, atoi(Symbol[k].data), buf);
		if (*Symbol[k].word_type == 'w')
			LC += 2;
		else if (*Symbol[k].word_type == 'b')
			LC += 1;
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
	/*
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

	*/

	fclose(in);

	getchar();
}