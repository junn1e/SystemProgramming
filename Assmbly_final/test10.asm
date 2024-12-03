mov	AX, 257
mov	CX, -1
mov	BL, 10

mov	AL, BL
mov	CL, AL
mov	BL, DL

mov	AX, BX
mov	CX, AX
mov	BX, DX
mov	SI, BX
mov	CX, DI

mov	AX, DATA1
mov	BX, DATA1

mov	AX, [BX]
mov	BX, [BP]
mov	EX, [SI]
mov	DX, [DI]

mov	DATA1, AX
mov	DATA1, BX

L1:
mov	[BX], AX
mov	[BP], BX
mov	[SI], CX
mov	[DI], DX

;
LOOP 	L1
JMP	L2
add	AX, 257
add	CX, -1
add	BL, 10

add	AX, BX
add	CX, AX
add	BX, DX
add	SI, BX
add	CX, DI

add	AX, DATA1
add	BX, DATA1

add	AX, [BX]
add	BX, [BP]
add	CX, [SI]
add	DX, [DI]

add	DATA1, AX
add	DATA1, BX

L2:
add	[BX], AX
add	[BP], BX
add	[SI], CX
add	[DI], DX

;
sub	AX, 257
sub	CX, -1
sub	BL, 10

L3:
sub	AX, BX
sub	CX, AX
sub	BX, DX
sub	SI, BX
sub	CX, DI

L4:
sub	AX, DATA1
sub	BX, DATA1

L5:
sub	AX, [BX]
sub	BX, [BP]
sub	CX, [SI]
sub	DX, [DI]

L6:
sub	DATA2, AX
sub	DATA2, BX

L7:
sub	[BX], AX
sub	[BP], BX
sub	[SI], CX
sub	[DI], DX

;
JZ	L3
JA	L4
JB	L5
JAE	L6
JBE	L7

;
INC	AX
INC	BX
INC	CX
INC	DX
DEC	AX
DEC	BX
DEC	CX
DEC	DX

INT 21

DATA1	DW	259
DATA2	DW	-1