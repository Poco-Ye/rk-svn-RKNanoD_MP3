	THUMB

	AREA |AudioCode|, CODE, READONLY, ALIGN=2
		        
	EXPORT SubFilter_Core_24

SubFilter_Core_24
	PUSH     {r4-r11}
	;PUSH     {r4-r10}
	ADD      r12,r2,r3,LSL #1  ;r2 g_FilterState结构体 r3 系数index*SHORT
	LDR      r4,[sp,#0x20]     ;
	LDRSH    r5,[r12,#0x2A0]    ;r5 = g_FilterState.a0
	LDRSH    r6,[r12,#0x2A2]    ;r6 = g_FilterState.a1
	LDRSH    r7,[r12,#0x2F4]    ;r7 = g_FilterState.b0
	LDRSH    r8,[r12,#0x2F6]    ;r8 = g_FilterState.b1
	ADD      r12,r4,r4,LSL #2  ;r4 = 5*L
	ADD      r12,r4,r12,LSL #2  ;r12 = 21*L
	ADD      r3,r3,r12,LSL #1  ;r3 = index +LR*21*2
	ADD      r10,r2,r3,LSL #3  ;r10 = g_FilterState.i_32buff[index +LR*42] 左移3因为long long类型
	SUB      r9,r0,#0x01       ;r9 = cwbuffer = process num	
	;lbuff_1 =  g_FilterState.i_32buff[index +LR*10]
    LDR      r4,[r10,#0x00]      ;  lbuff_1_L =r4
	LDR      r0,[r10,#0x04]      ;  lbuff_1_H =r0
	LDR      r11,[r10,#0x08]	   ;	lbuff_2_L =r11
	LDR      r12,[r10,#0x0C]     ;	lbuff_2_H =r12 

	;vincent think this is no necessary	
	;ADDS     r0,r0,#0
	;BEQ      exit

	;MOVW     r11,#0x7FFF
	;空闲寄存器 r2 r3 
begin
	;0-------------------------------
	LDR      r3,[r1,#0x00]     ;r3 = x1=(long )x 	
	MOV      r2,#0x2000
	SMLAL    r4,r0,r3,r2   ;y =lbuff_1 + ((unsigned long long)x<<13);
	LSR      r4,r4,#13; 	           
	ORR      r4,r4,r0,LSL #19   ; //逻辑or算术
	ASRS     r0,r0,#13          ; y =y>>13
	
	MOV      r2,r4             ;r2 = y1 = y

	SSAT	 r4 , #24 , r4       ;取24bit的r0

	STR      r4,[r1],#0x08     ;输出 y 存储字，r1 偏移4个字节(单声道处理 pwbuffer += mode;)
	SMULL    r4,r0,r3,r7       ;lbuff_1 = x1 * b0
	SMLAL    r4,r0,r2,r5       ;lbuff_1 = y1*a0 +x1*b0
	ADDS     r4,r4,r11
	ADCS     r0,r0,r12         ;r4 = lbuff_1 = lbuff_2 +x1*b0 +y1*a0
	SMULL    r11,r12,r3,r8     ;r12 11 = x1*b1
	SMLAL    r11,r12,r2,r6     ;r12 11= lbuff_2 = x1*b1+y1*a1

	;1-------------------------------
	LDR    r3,[r1,#0x00]
	MOV      r2,#0x2000
	SMLAL    r4,r0,r3,r2
	LSR      r4,r4,#13; 
	ORR      r4,r4,r0,LSL #19
	ASRS     r0,r0,#13 
	MOV      r2,r4 
    SSAT	 r4 , #24 , r4

	STR      r4,[r1],#0x08
	SMULL    r4,r0,r3,r7 
	SMLAL    r4,r0,r2,r5
	ADDS     r4,r4,r11
    ADCS     r0,r0,r12  
	SMULL    r11,r12,r3,r8
	SMLAL    r11,r12,r2,r6


	;2-------------------------------
	LDR    r3,[r1,#0x00]
	MOV      r2,#0x2000
	SMLAL    r4,r0,r3,r2
	LSR      r4,r4,#13; 
	ORR      r4,r4,r0,LSL #19
	ASRS     r0,r0,#13 
	MOV      r2,r4 
    SSAT	 r4 , #24 , r4

	STR      r4,[r1],#0x08
	SMULL    r4,r0,r3,r7 
	SMLAL    r4,r0,r2,r5
	ADDS     r4,r4,r11
    ADCS     r0,r0,r12  
	SMULL    r11,r12,r3,r8
	SMLAL    r11,r12,r2,r6


	;3-------------------------------
	LDR    r3,[r1,#0x00]
	MOV      r2,#0x2000
	SMLAL    r4,r0,r3,r2 
	LSR      r4,r4,#13; 
	ORR      r4,r4,r0,LSL #19
	ASRS     r0,r0,#13 
	MOV      r2,r4 
    SSAT	 r4 , #24 , r4

	STR      r4,[r1],#0x08
	SMULL    r4,r0,r3,r7 
	SMLAL    r4,r0,r2,r5
	ADDS     r4,r4,r11
    ADCS     r0,r0,r12  
	SMULL    r11,r12,r3,r8
	SMLAL    r11,r12,r2,r6


	;4-------------------------------
	LDR    r3,[r1,#0x00]
	MOV      r2,#0x2000
	SMLAL    r4,r0,r3,r2 
	LSR      r4,r4,#13; 
	ORR      r4,r4,r0,LSL #19
	ASRS     r0,r0,#13 
	MOV      r2,r4 
    SSAT	 r4 , #24 , r4

	STR      r4,[r1],#0x08
	SMULL    r4,r0,r3,r7 
	SMLAL    r4,r0,r2,r5
	ADDS     r4,r4,r11
    ADCS     r0,r0,r12  
	SMULL    r11,r12,r3,r8
	SMLAL    r11,r12,r2,r6


	;5-------------------------------
	LDR    r3,[r1,#0x00]
	MOV     r2,#0x2000
	SMLAL    r4,r0,r3,r2
	LSR      r4,r4,#13; 
	ORR      r4,r4,r0,LSL #19
	ASRS     r0,r0,#13 
	MOV      r2,r4 
    SSAT	 r4 , #24 , r4

	STR      r4,[r1],#0x08
	SMULL    r4,r0,r3,r7 
	SMLAL    r4,r0,r2,r5
	ADDS     r4,r4,r11
    ADCS     r0,r0,r12  
	SMULL    r11,r12,r3,r8
	SMLAL    r11,r12,r2,r6


	;6-------------------------------
	LDR    r3,[r1,#0x00]
	MOV      r2,#0x2000
	SMLAL    r4,r0,r3,r2 
	LSR      r4,r4,#13; 
	ORR      r4,r4,r0,LSL #19
	ASRS     r0,r0,#13 
	MOV      r2,r4 
    SSAT	 r4 , #24 , r4

	STR      r4,[r1],#0x08
	SMULL    r4,r0,r3,r7 
	SMLAL    r4,r0,r2,r5
	ADDS     r4,r4,r11
    ADCS     r0,r0,r12  
	SMULL    r11,r12,r3,r8
	SMLAL    r11,r12,r2,r6


	;7-------------------------------
	LDR   r3,[r1,#0x00]
	MOV      r2,#0x2000
	SMLAL    r4,r0,r3,r2 
	LSR      r4,r4,#13; 
	ORR      r4,r4,r0,LSL #19
	ASRS     r0,r0,#13 
	MOV      r2,r4 
    SSAT	 r4 , #24 , r4

	STR      r4,[r1],#0x08
	SMULL    r4,r0,r3,r7 
	SMLAL    r4,r0,r2,r5
	ADDS     r4,r4,r11
    ADCS     r0,r0,r12  
	SMULL    r11,r12,r3,r8
	SMLAL    r11,r12,r2,r6

	;SUBS     r9,r9,#0x01
	SUBS     r9,r9,#0x08
	BCS      begin

exit
	STR      r4,[r10,#0x00]  ;  buff1_L =r4
	STR      r0,[r10,#0x04]  ;  buff1_H =r6
	STR      r11,[r10,#0x08]	   ;	buff2_L =r11
	STR      r12,[r10,#0x0C]    ;	buff2_H =r12
	POP      {r4-r11}
	;POP      {r4-r10}
	BX       lr

	END