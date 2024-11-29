#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>

// 레지스터 테이블 구조체
struct reg
{
	char reg_name[3];
	char reg_num[4];
} Reg[20];
// 심볼 테이블 구조체
struct symbol_tbl
{
	char symbol[10];
	char word_type[2];
	int lc;
	char data[10];
} Symbol[20];
// 명령어 테이블 구조체
struct ins
{
	char instruct[6];
	char dest[2];
	char sour[2];
	char word_type[2];
	char ins_code[3];
	char ins_len[2];
	char mod_reg[9];
} Instr[100];
// 라벨 구조체
struct sentence
{
	char label[10];
	char _operator[10];
	char operand[3][10];
} Sen;

// 최대 명령어, 심볼, 라벨 개수
int MaxI, MaxS, LC;
#define MAX_INS 1     // 명령어의 최대 개수를 지정

// 하는 일 : 이진수를 정수로 변환
int btoi(char *dig)
{
	register int i = 0, ret = 0; // i는 인덱스, ret는 반환값
	while (*(dig + i) != '\0') // 문자열 끝까지 반복
	{
		if (*(dig + i) == '1') // '1'인 경우 해당 위치의 2의 제곱값을 더함
			ret += pow((double)2, (double)strlen(dig + i) - 1);
		i++; // 다음 문자로 이동
	}
	return ret; // 변환된 정수 반환
}

// 하는 일 : 파일에서 레지스터와 명령어 정보를 읽어 테이블에 저장
void Initialize()
{
	int i=0, j=1;
	FILE *regi, *inst;
	regi=fopen("reg_tbl.txt","r");
	inst=fopen("inst_tbl.txt","r");
	if( !regi || !inst ){
		printf("파일 열기 실패 : Initialize()");
		return;
	}
	while(!feof(regi))
	{
		fscanf(regi,"%s%s\n", Reg[i].reg_name, Reg[i].reg_num);
		i++;
	}     // 레지스터 테이블을 작성한다
	while(!feof(inst))
	{
		fscanf(inst, "%6s%2s%2s%4s%3s%2s%9s\n", Instr[j].instruct, 
			Instr[j].dest, Instr[j].sour, 
			Instr[j].word_type, Instr[j].ins_code, 
			Instr[j].ins_len, Instr[j].mod_reg);
		j++;
	}     // 명령어 테이블을 작성한다.
	MaxI=j-1;
	printf("테이블 작성 완료\n");
	fclose(regi);
	fclose(inst);
}

// 하는 일 : 문자열의 종류 반환 (0:주소값, 1:2바이트 reg, 2:1바이트 reg, 3:라벨)
int Analyze(char *operand)
{
	int i=0;
	char *regist[]={"AX","BX","CX","DX","AL","BL","CL","DL","AH","BH","CH","DH",0x00};     // 레지스터의 이름을 저장

	// 문자열이 숫자인지 확인 (즉시지정)
	if(isdigit(operand[0])) return 0; // 숫자(즉시지정)
	
	// 문자열이 레지스터인지 확인
	while(regist[i]!=0x00){
		if(!strcmp(operand,regist[i]))
		{
			if(i<4) return 1; // 2바이트 레지스터
			else return 2;    // 1바이트 레지스터
		}
		else i++;
	}
	return 3; // 라벨
}

// 하는 일 : 오퍼랜드의 어드레스 모드를 판정
int Add_Chk(char *sen)
{
	register int k=MaxI;
	int i=0,j=0,l=0,wp=0;
	char op[5][10],*opcode[]={"mov",""};
	while(sen[wp]!='\n')
	{
		while(sen[wp]==' '||sen[wp]=='\t'||sen[wp]==',') wp++;     // 공백, 탭, 콤마는 통과

		while(sen[wp]!=' '&&sen[wp]!='\t' &&sen[wp]!='\n' &&sen[wp]!=',')
		{
			*(op[j]+i)=sen[wp];
			i++;
			wp++;
		}
		*(op[j]+i)='\0';
		i=0;
		j++;
	}
	i=0;
	while(strcmp(opcode[i],""))
		if(stricmp(opcode[i],op[0]))i++;
		else
		{
			strcpy(Sen._operator,op[0]);
			for(l=1; l<j; l++) strcpy(Sen.operand[l-1],op[l]);
			break;
		}
	if(i==MAX_INS)
	{
		strcpy(Sen.label,op[0]);
		strcpy(Sen._operator,op[1]);

		for(l=2;l<j;l++) strcpy(Sen.operand[l-2],op[1]);
	}     // 한 문장을 분석하여 레이블, 오퍼레이터와 오퍼랜드로 분류한다
	strcpy(Instr[0].instruct,op[0]);     // OP코드를 저장, bug????

	switch(Analyze(op[1]))
	{
		case 0: strcpy(Instr[0].dest,"i");
			break;
		case 1: strcpy(Instr[0].dest,"r");
			strcpy(Instr[0].word_type,"w");
			break;
		case 2: strcpy(Instr[0].dest,"r");
			strcpy(Instr[0].word_type,"b");
			break;
		case 3: strcpy(Instr[0].dest,"m");
			break;
	}
	switch(Analyze(op[2]))
	{
		case 0: strcpy(Instr[0].sour,"i");
			break;
		case 1: strcpy(Instr[0].sour,"r");
			strcpy(Instr[0].word_type,"w");
			break;
		case 2: strcpy(Instr[0].sour,"r");
			strcpy(Instr[0].word_type,"b");
			break;
		case 3: strcpy(Instr[0].sour,"m");
			break;
	}

	while(stricmp(Instr[k].instruct,Instr[0].instruct)
			||strcmp(Instr[k].dest,Instr[0].dest)
			||strcmp(Instr[k].sour,Instr[0].sour)
			||strcmp(Instr[k].word_type,Instr[0].word_type))
			k--;
	return k; // k == 0 : symbol
}

// 하는 일 : 첫번째 읽기
void PassI(char *buf)
{
	int i;
	static int j=0;
	i=Add_Chk(buf);

	if(i)
	{
		printf("%04X: %s",LC,buf);
		LC+=atoi(Instr[i].ins_len);
	}
	else
	{
		if(!stricmp(Sen._operator,"dw"))
			strcpy(Symbol[j].word_type,"w");
		else if(!stricmp(Sen._operator,"db"))
			strcpy(Symbol[j].word_type,"b");
		strcpy(Symbol[j].symbol,Sen.label);
		strcpy(Symbol[j].data,Sen.operand[0]);
		Symbol[j].lc=LC;
		printf("%04X: %s",LC,buf);
		if(*Symbol[j].word_type =='w')   LC+=2;
		else if(*Symbol[j].word_type == 'b') LC+=1;
		j++;
	}
}

// 하는 일 : 두번쨰 읽기
void PassII(char *buf)
{
	int i, j=0, k=0;

	// 오퍼랜드의 어드레스 모드를 판정
	i=Add_Chk(buf);
	
	if(i) // 즉시지정이 아닐 때
	{
		// 명령어 코드 출력
		printf("%04x: %3s", LC, Instr[i].ins_code);

		// 대상이 레지스터인 경우 -> 대상 레지스터 번호 복사
		if(!strcmp(Instr[i].dest,"r"))
		{
			while(stricmp(Reg[j].reg_name,Sen.operand[0]))
				j++;
			strncpy(strchr(Instr[0].mod_reg,'?'),Reg[j].reg_num,3);
		}

		j=0;
		// 소스가 레지스터인 경우 -> 소스 레지스터 번호 복사
		if(!strcmp(Instr[i].sour,"r"))
		{
			while(stricmp(Reg[j].reg_name,Sen.operand[1]))
				j++;
			strncpy(strchr(Instr[0].mod_reg,'?'),Reg[j].reg_num,3);
		}

		// 대상과 소스 모두 메모리가 아닌 경우
		if(strcmp(Instr[i].dest,"m")&&strcmp(Instr[i].sour,"m")){
			printf("%02X %s",btoi(Instr[i].mod_reg),buf);
		}
		// 메모리 연산이 포함된 경우
		else
		{
			// 메모리 번호 복사
			if(!strcmp(Instr[i].dest,"m"))
				while(strcmp(Symbol[k].symbol,Sen.operand[0]))k++;
			else if(!strcmp(Instr[i].sour,"m"))
				while(strcmp(Symbol[k].symbol,Sen.operand[1]))k++;

			printf(" %02X %04X %s", btoi(Instr[i].mod_reg),Symbol[k].lc,buf);
		}
		LC+=atoi(Instr[i].ins_len);
	}

	else // 주소즉시지정일 때
	{
		k=0;
		while(strcmp(Symbol[k].symbol,Sen.label))k++;
		if(!strcmp(Symbol[k].word_type,"w"))
			printf("%04X:%04X %20s", LC, atoi(Symbol[k].data),buf);
		if(!strcmp(Symbol[k].word_type,"b"))
			printf("%04X: %02X %20s",LC,atoi(Symbol[k].data),buf);
		if(*Symbol[k].word_type == 'w')  LC+=2;
		else if(*Symbol[k].word_type == 'b') LC+=1;
	}
}

void main()
{
	char buf[50];
	FILE *in;
	in=fopen("test1.asm", "r");
	if( !in ) {
		printf("파일 열기 실패 : main()");
		return;
	}
	Initialize();
	printf("\nPass1:\n");
	while(1)
	{
		fgets(buf,30,in);
		if(feof(in))break;
		PassI(buf);
	}
	rewind(in);
	LC=0;
	printf("\nPass2:\n");
	
	while(1)
	{
		fgets(buf,30,in);
		if(feof(in)) break;
		PassII(buf);
	}
	fclose(in);

	getchar();
}