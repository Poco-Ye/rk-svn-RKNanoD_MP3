  THUMB

	AREA |AudioCode|, CODE, READONLY, ALIGN=2
		        
	EXPORT filter_2_int_r
	
filter_2_int_r
	
  PUSH   {r0-r1,r4-r11,lr}
	MOV    r4,r0;  
	SUB    r9,r1,#0x01     
	EXTERN  x_r;
	EXTERN  y_r;
	LDR     r0,= x_r;   
	LDRSH    r10,[r0,#0x00] ; x1 = r10 
	LSL      r3,r10,#16;
	LSR      r10,r3,#16;     
	LDRSH    r11,[r0,#0x02]  ;x2 = r11 
	ORR      r10,r10,r11,LSL #16;
	LDR      r0,= y_r;   
	LDR      r5,[r0,#0x00]  ;  y1_L =r5
	LDR      r6,[r0,#0x04]  ;  y1_H =r6
	LDR      r7,[r0,#8]	  ;	   y2_L =r7
	LDR      r8,[r0,#12]  ;	   y2_H =r8
	SUB      sp,#0x8;

	;temp register  r0,r1,r2,r3,r12
begin
	
	;1------------------------------------  
	MOV      r0,#0x2000;   ; 	b0 = r0
	LDRSH    r2,[r4,#0x00] ;    x0 = r2
	
	LSL      r12,r2,#16 ;
	LSR      r12,r12,#16	; save unsigned x0 for new x1
;	MOV      r12,r2     ;       x1 = x0	= r12 
	
	MUL     r2 ,r0,r2;          b0*x0 =	r2
	
	;2-------------------------------------
	MOV      r0,#0xc000;   ;  	 b1 = r0
	LSL      r0,#16 ;
	ASR      r0,#16		   ;
    LSL      r3,r10,#16 ;
	ASR      r3,r3,#16         ; get x1 from r10 ,save x1 for new x2
	MUL      r0,r3,r0;          b1*x1  = r0
	ASRS     r1,r0,#31;   
	ADDS      r0,r0,r2 	 ;		 b0*x0+b1*x1 = r0;
	ADC      r1,r1,r2,ASR #31   ; add flag =  r1;   
	MOV      r2,r0              ; b0*x0+b1*x1 =r2; 

	;3--------------------------------------
	MOV      r0,#0x2000;         b2 = r0 ;  
	ASR      r11,r10,#16	;   get  x2 from r10
	ORR      r10,r12,r3,LSL #16;  save new x2 ¡¢x1 that is x1,x0
	MUL     r0 ,r11,r0;          b2*x2 = r0  ;
	ASR     r11,r0,#31;           *flag =  r3 
	ADDS      r3,r0,r2 		   ;  L b0*x0+b1*x1 +b2*x2 = r0
	ADC      r11,r1,r11  		   ;  H b0*x0+b1*x1 +b2*x2 = r1
		
;	MOV      r11,r3;         x2= x1;
;	MOV      r10,r12;         x1 = x0;
;	STRD     r0,r1,[sp,#0x00];    store x[] *b[]  result
	;4-------------------------------------------//
	MOV      r2,#0x3fef;    ;	  a1 = r2;

	UMULL    r0 ,r12,r2,r5;       a1*y1L  = r12 r0	L
	ASR      r1,r2,#31;           a1 flag = r1
	MLA      r1,r1,r5,r12; 		  a1 flag * y1L + carry bit = r1 ;
	MLA      r1,r2,r6,r1;  L	  a1 *y1H + r1 carry bit = r1   H
  
		 
    SUBS     r1,#0;		          r1 r0  > 0
	BGE      G_OR_EQ;			  if >0 goto 	 G_OR_EQ
	MVN      r1,r1		        ; if  <0
	MVN      r0,r0				 ; ~ r1 r0 
    ADDS     r0,r0,#1; 			 
	ADC      r1,r1,#0 			;  +1
	LSR     r0,r0,#13; 	  		;
	ORR      r0,r0,r1,LSL #19   ; r0>>13 xor  r1 << 19 = new r0	 
	ASRS     r1,r1,#13;   		 ;r1 >> 13             = new r1
	MVN      r1,r1
	MVN      r0,r0
	ADDS     r0,r0,#1; 
	ADC      r1,r1,#0 
	B        ALL
G_OR_EQ    
	LSR     r0,r0,#13; 	          r0>>13 xor  r1 << 19 = new r0	 
	ORR      r0,r0,r1,LSL #19   ; r1 >> 13             = new r1
	ASRS     r1,r1,#13;  
ALL	
	;LDRD     r2,r3,[sp,#0x00]	;
	ADDS     r3,r0,r3
	ADCS     r11,r1,r11;  		; 
;	STRD     r0,r1,[sp,#0x00]   ;  store 4 plus result
	;5-------------------------------------------- 
	MOV      r12,#0xe011;        a2 = r12;
    LSL      r12,#16 ;
	ASR      r12,#16 
	UMULL     r0,r2,r12,r7		; a2 * y2L  = r2 r0	  L
	ASR      r1,r12,#31; 	    ; a2 flag = r1
	MLA      r1,r1,r7,r2; 		; a2 flag *y2L + carry = r1
	MLA      r1,r12,r8,r1	    ; a2 * y2H + r1 carry bit =	r1	  H
    SUBS     r1,#0;				; right shift 13  
	BGE      G_OR_EQ_1;
	MVN      r1,r1
	MVN      r0,r0
    ADDS     r0,r0,#1; 
	ADC      r1,r1,#0 
	LSR     r0,r0,#13; 	 x
	ORR      r0,r0,r1,LSL #19   ;	 x
	ASRS     r1,r1,#13;   
	MVN      r1,r1
	MVN      r0,r0
	ADDS     r0,r0,#1; 
	ADC      r1,r1,#0 
	

	
	
	B        ALL1
G_OR_EQ_1    
	LSR     r0,r0,#13; 	 x
	ORR      r0,r0,r1,LSL #19   ;	 x
	ASRS     r1,r1,#13; 
ALL1	
	
;	LDRD     r2,r3,[sp,#0x00]
	ADDS     r0,r0,r3
	ADCS     r1,r1,r11;  		;	  5 plus result
	
	;--------------------
	
	MOV      r7,r5;
	MOV      r8,r6;       y2=y1;
	MOV      r5,r0;
	MOV      r6,r1;       y1=y0;
	

    SUBS     r1,#0;			    ;  abs(y0)< 32768  y0 >> 13    
	BGE      G_OR_EQ_2;
	CMP      r1,#0xFFFFFFFF;
    BNE      peak1
	MOV      r2,r0;
	LSR      r2,#28;
	CMP      r2,#0xF;   
	BNE      peak1;
	MVN      r1,r1
	MVN      r0,r0
    ADDS     r0,r0,#1; 
	ADC      r1,r1,#0 
	LSR      r0,r0,#13; 	 
	ORR      r0,r0,r1,LSL #19  ;	
	ASRS     r1,r1,#13;   	
	MVN      r0,r0
	ADDS     r0,r0,#1; 	
	B        ALL2
peak1
    MOV      r0,#0x8000;   FFFF8000
	LSL      r0,#16 ;
	ASR      r0,#16
    B        ALL2;
G_OR_EQ_2 
    CMP      r1,#0;
    BNE      peak2
	MOV      r2,r0;
	LSR      r2,#28;
	CMP      r2,#0;   
	BNE      peak2;
	LSR     r0,r0,#13;
	ORR      r0,r0,r1,LSL #19 
	ASRS     r1,r1,#13;   
	B        ALL2;  
peak2
   MOV      r0,#0x7FFF;
    B        ALL2;
ALL2 
	
	STRH     r0,[r4],#0x00
	ADDS      r4,r4,#4  
	;---------------------
	SUBS     r9,r9,#0x01 
	BCS      begin     ;
	

    EXTERN  x_r
	EXTERN  y_r
	LDR     r0,= x_r;   
	LSL      r3,r10,#16 ;
	ASR      r3,r3,#16	 
	STRH    r3,[r0,#0x00] ;
;	STRH    r10,[r0,#0x00] ;
	ASR      r11,r10,#16	 
	STRH     r11,[r0,#0x02]   ;
	LDR      r0,= y_r;   
	STR      r5,[r0,#0x00]   
	STR      r6,[r0,#0x04]   
	STR      r7,[r0,#8]
	STR      r8,[r0,#12]  ;
	ADD      sp,#0x08;
	POP      {r0-r1,r4-r11,lr} ;
	BX       lr
	
	END	