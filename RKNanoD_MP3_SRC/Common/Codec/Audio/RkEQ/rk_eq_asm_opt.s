	THUMB

	AREA |AudioCode|, CODE, READONLY, ALIGN=2

	EXPORT SubFilter_Core

SubFilter_Core
	PUSH     {r4-r11}
	;PUSH     {r4-r10}
	ADD      r12,r2,r3,LSL #1
	LDR      r4,[sp,#0x20]
    CBNZ     r4, r4_eq_1
	LDRSH    r5,[r12,#0x60]
	LDRSH    r6,[r12,#0x62]
	LDRSH    r7,[r12,#0x74]
	LDRSH    r8,[r12,#0x76]
r4_eq_1
    LDRSH    r5,[r12,#0x88]
	LDRSH    r6,[r12,#0x8A]
	LDRSH    r7,[r12,#0x9C]
	LDRSH    r8,[r12,#0x9E]
	ADD      r12,r4,r4,LSL #2
	ADD      r3,r3,r12,LSL #1
	ADD      r10,r2,r3,LSL #2
	SUB      r9,r0,#0x01
	LDR      r12,[r10,#0x08]
	LDR      r4,[r10,#0x04]

	;vincent think this is no necessary
	;ADDS     r0,r0,#0
	;BEQ      exit

	;MOVW     r11,#0x7FFF
begin
	;7-------------------------------
	LDRSH    r3,[r1,#0x00]
	ADD      r0,r4,r3,LSL #13
	ASR      r0,r0,#13

	MOV      r2,r0

	SSAT	 r0 , #16 , r0

	STRH     r0,[r1],#0x04
	MUL      r0,r3,r7
	MLA      r0,r2,r5,r0
	ADD      r4,r0,r12
	MUL      r0,r3,r8
	MLA      r12,r2,r6,r0
	;SUBS     r9,r9,#0x01
	SUBS     r9,r9,#0x01
	BCS      begin

exit
	STR      r12,[r10,#0x08]
	STR      r4,[r10,#0x04]
	POP      {r4-r11}
	;POP      {r4-r10}
	BX       lr

	END