  THUMB

	AREA |AudioCode|, CODE, READONLY, ALIGN=2
		        
	EXPORT SubFilter_bass_Core_24
	
SubFilter_bass_Core_24   ; SubFilter_bass_Core(long *pwBuffer, unsigned short frameLen,int index,int L)
	
  PUSH   {r0-r1,r4-r11,lr};
	MOV    r4,r0; r4 ╩¤╛▌╘┤
	SUB    r9,r1,#0x01  ;  
	EXTERN g_bass_FilterState;
	LDR     r0,= g_bass_FilterState;   
	ADD      r12,r3,r3,LSL #2 ; r12= 5*L
	ADD      r3,r2,r12,LSL #1 ; r3 = 10*L+index
	ADD      r10,r0,r3,LSL #4 ; r10 = g_bass_FilterState + (20*L+ index*2)*8
		
	LDR      r5,[r10,#0x00]  ;  i_32buff1_L =r5
	LDR      r6,[r10,#0x04]  ;  i_32buff1_H =r6
	LDR      r7,[r10,#0x08]	   ;	i_32buff2_L =r7
	LDR      r8,[r10,#0x0C]    ;	i_32buff2_H =r8
	
	ADD      r12,r2,r2,LSL #2 ; r12= 5*index
	ADD      r12,r0,r12,LSL #2        ; r12 = g_bass_FilterState + index*5*4
	;    [r12,#0x140]  ;   a1 
	;    [r12,#0x144]  ;   a2 
	;    [r12,#0x148]	   ;   b0
	;    [r12,#0x14C]    ;	 b1 
  ;    [r12,#0x150]    ;	 b2

	
	SUB      sp,#0x8; 

	;temp ?? register  r0,r1,r2,r3,r11
begin
	
	;1------------???? y0 ------------------------  
	LDR      r0,[r12,#0x148]	  ; 	b0 = r0
	LDR      r11,[r4,#0x00]    ;   x0 = r11	 
	SMLAL    r5, r6 ,r0,r11         ;  b0*x0 + buff_1=	y0= r3 r2
	LSR      r5,r5,#10; 
	ORR      r5,r5,r6,LSL #22
	ASRS     r6,r6,#10               ;y0 >>10
        MOV      r2,r5;            
	MOV      r3,r6;            	 
	
	;2------------???? a1*y0 + buffer_2-------(?иж?Yио?5?бщ6;2?бщ32??и╣ио?)------------------
	LDR      r0,[r12,#0x140]	  ; 	a1 = r0 ;	

	UMULL    r5 ,r6,r0,r2;       a1*y0L  = r6 r5	L
	ASR      r1,r0,#31;           a1 flag = r3
	MLA      r1,r1,r2,r6; 		  a1 flag * y0L + carry bit = r1 ;
	MLA      r1,r0,r3,r1;  L	  a1 *y0H + r1 carry bit = r1   H
        MOV      r0,r5       ;      r1 H  r0  L		 
    
	LSR     r0,r0,#10; 	          r0>>10 xor  r1 << 22 = new r0	 
	ORR      r0,r0,r1,LSL #22   ; r1 >> 10            = new r1
	ASRS     r1,r1,#10;  
	

ALL	

	ADDS     r5,r0,r7
	ADCS     r6,r1,r8;     ;    a1*y0 + buffer_2
	
	;3------------buff_1 = b1 *x0+ a1*y0 + buffer_2---------------------------
    LDR      r0,[r12,#0x14C]    ;	 b1 =r0
	SMLAL    r5, r6 ,r0,r11         ;  buff_1=r6 r5  
	 	
	

	 	
	;4------------???? a2*y0  -------(2?бщ3?бщ4?бщ5 6?бщ9?бщ?бщ11?бщ122??и╣ио?;7?бщ8 ?иж?Yио?)------------------
	LDR      r0,[r12,#0x144]	  ; 	a2 = r0 ;	
	UMULL    r7 ,r8,r0,r2;       a2*y0L  = r7 r8	L
	ASR      r1,r0,#31;           a1 flag = r3
	MLA      r1,r1,r2,r8; 		  a2 flag * y0L + carry bit = r1 ;
	MLA      r1,r0,r3,r1;  L	  a2 *y0H + r1 carry bit = r1   H
    MOV      r0,r7       ;      r1 H  r0  L
		 
   
	LSR     r0,r0,#10; 	          r0>>10 xor  r1 << 22 = new r0	 
	ORR      r0,r0,r1,LSL #22   ; r1 >> 10            = new r1
	ASRS     r1,r1,#10;  	

ALL1	

	MOV     r7,r0;
	MOV     r8,r1;     ;    a1*y0 + buffer_2


;5 -------------buff_2 = b2  *x0 +a2*y0-----------------------
   	LDR      r0,[r12,#0x150]    ;	 b2 =r0
	SMLAL    r7, r8 ,r0,r11   ;  buff_2=r8 r7  
;-----    ---------------
	
	MOV      r0,r2;
	MOV      r1,r3;

	

    SUBS     r1,#0;			    ;  abs(y0)< 32768  y0 >> 20    
	BGE      G_OR_EQ_2;
    
	
	MOV      r2,r1;
	ASR      r2,#1;    10+ 23 - 32	  if process value >>(low 32) ,need process low 32bit(r0),not high 32bit
	CMP      r2,#0xFFFFFFFF;   
	BNE      peak1;

    LSR      r0,r0,#10;
	ORR      r0,r0,r1,LSL #22 
	ASRS     r1,r1,#10;  	
	B        ALL2
peak1
    MOV      r0,#0x800000;   FFFF8000  0xFF800000
	LSL      r0,#8 ;
	ASR      r0,#8
    B        ALL2;
G_OR_EQ_2 

	MOV      r2,r1;
	LSR      r2,#1;  10+ 23 - 32
	CMP      r2,#0;   
	BNE      peak2;

	LSR      r0,r0,#10;
	ORR      r0,r0,r1,LSL #22 
	ASRS     r1,r1,#10;   

	B        ALL2;

peak2
   MOV      r0,#0x800000;
   SUB      r0,#1;
    B        ALL2;
ALL2 

	STR      r0,[r4],#0x00   ;
	ADDS      r4,r4,#8  
	;---------------------
	SUBS     r9,r9,#0x01 ;
	BCS      begin     ;
	

		
	STR      r5,[r10,#0x00]  ;  i_32buff1_L =r5
	STR      r6,[r10,#0x04]  ;  i_32buff1_H =r6
	STR      r7,[r10,#0x08]	   ;	i_32buff2_L =r7
	STR      r8,[r10,#0x0c]    ;	i_32buff2_H =r8
    
	ADD      sp,#0x08;
	POP      {r0-r1,r4-r11,lr} ;
	
	BX       lr
	
	END	