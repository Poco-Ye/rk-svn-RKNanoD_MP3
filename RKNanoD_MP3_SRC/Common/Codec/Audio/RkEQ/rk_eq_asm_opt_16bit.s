	THUMB

	AREA |AudioCode|, CODE, READONLY, ALIGN=2
		        
	EXPORT SubFilter_Core_16

SubFilter_Core_16		  ;SubFilter_Core(cwbuffer,pwbuffer, &g_FilterState , (index<<1) , LR );
	PUSH     {r4-r11}
	;PUSH     {r4-r10}
	ADD      r12,r2,r3,LSL #1	;  g_FilterState+ 2*index*2	   换算到字节
	LDR      r4,[sp,#0x20]	; 5段
	LDRSH    r5,[r12,#0x150]	;5段   #0x60 	#0x62	 #0x74	  0x76
	LDRSH    r6,[r12,#0x152]	;21段   #0x160 	#0x160	 #0x1B4	  0x1B6
	LDRSH    r7,[r12,#0x1A4]
	LDRSH    r8,[r12,#0x1A6]
	ADD      r12,r4,r4,LSL #2	       ; 5*LR;原5*LR	                            ADD      r12,r4,r4,LSL #2
	ADD      r12,r4,r12,LSL #2		   ;5*4*LR+LR
	ADD      r3,r3,r12,LSL #1	  ;index+ 42*LR;原index + 10*LR                 ADD      r3,r3,r12,LSL #1
	ADD      r10,r2,r3,LSL #2	  ; g_FilterState + (index + 42*LR)*4 ;原g_FilterState + (index*2 + 10*LR)*4    ADD      r10,r2,r3,LSL #2
	SUB      r9,r0,#0x01
	LDR      r12,[r10,#0x04]	 ;buff2
	LDR      r4,[r10,#0x00]		 ;buff1

	;vincent think this is no necessary	
	;ADDS     r0,r0,#0
	;BEQ      exit

	;MOVW     r11,#0x7FFF
begin
	;0-------------------------------
	LDRSH    r3,[r1,#0x00]
	ADD      r0,r4,r3,LSL #13
	ASR     r0,r0,#13
	
	MOV      r2,r0

	SSAT	 r0 , #16 , r0

	STRH     r0,[r1],#0x04
	MUL      r0,r3,r7
	MLA      r0,r2,r5,r0
	ADD      r4,r0,r12
	MUL      r0,r3,r8
	MLA      r12,r2,r6,r0
	;SUB      r9,r9,#0x01
	;BCS      begin

	;1-------------------------------
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
	;SUB      r9,r9,#0x01
	;BCS      begin

	;2-------------------------------
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
	;SUB      r9,r9,#0x01
	;BCS      begin

	;3-------------------------------
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
	;SUBS     r9,r9,#0x04
	;BCS      begin

	;4-------------------------------
	LDRSH    r3,[r1,#0x00]
	ADD      r0,r4,r3,LSL #13
	ASR     r0,r0,#13
	
	MOV      r2,r0

	SSAT	 r0 , #16 , r0

	STRH     r0,[r1],#0x04
	MUL      r0,r3,r7
	MLA      r0,r2,r5,r0
	ADD      r4,r0,r12
	MUL      r0,r3,r8
	MLA      r12,r2,r6,r0
	;SUB      r9,r9,#0x01
	;BCS      begin

	;5-------------------------------
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
	;SUB      r9,r9,#0x01
	;BCS      begin

	;6-------------------------------
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
	;SUB      r9,r9,#0x01
	;BCS      begin

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
	SUBS     r9,r9,#0x08
	BCS      begin

exit
	STR      r12,[r10,#0x04]
	STR      r4,[r10,#0x00]
	POP      {r4-r11}
	;POP      {r4-r10}
	BX       lr

	END