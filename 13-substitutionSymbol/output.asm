mov	AX, 257mov	CX, -1mov	BL, 10
mov	AL, BLmov	CL, ALmov	BL, DL
mov	AX, BXmov	CX, AXmov	BX, DXmov	SI, BXmov	CX, DI
mov	AX, DATA1mov	BX, DATA1
mov	AX, [BX]mov	BX, [BP]mov	EX, [SI]mov	DX, [DI]
mov	470, AXmov	470, BX
L1:mov	[BX], AXmov	[BP], BXmov	[SI], CXmov	[DI], DX
;LOOP 	L1JMP	L2add	AX, 257add	CX, -1add	BL, 10
add	AX, BXadd	CX, AXadd	BX, DXadd	SI, BXadd	CX, DI
add	AX, DATA1add	BX, DATA1
add	AX, [BX]add	BX, [BP]add	CX, [SI]add	DX, [DI]
add	470, AXadd	470, BX
L2:add	[BX], AXadd	[BP], BXadd	[SI], CXadd	[DI], DX
;sub	AX, 257sub	CX, -1sub	BL, 10
L3:sub	AX, BXsub	CX, AXsub	BX, DXsub	SI, BXsub	CX, DI
L4:sub	AX, DATA1sub	BX, DATA1
L5:sub	AX, [BX]sub	BX, [BP]sub	CX, [SI]sub	DX, [DI]
L6:sub	476, AXsub	476, BX
L7:sub	[BX], AXsub	[BP], BXsub	[SI], CXsub	[DI], DX
;JZ	L3JA	L4JB	L5JAE	L6JBE	L7
;INC	AXINC	BXINC	CXINC	DXDEC	AXDEC	BXDEC	CXDEC	DX
INT 21
DATA1	DW	259476	DW	-1
;