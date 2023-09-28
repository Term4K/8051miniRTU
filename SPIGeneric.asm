.module SPIGeneric
.globl __spigeneric_frame_start
.globl __spigeneric_frame_end
.globl __spigeneric_spit_bit
.globl __spigeneric_spit_byte
.globl __spigeneric_reset
.area RSEG    (ABS,DATA)
.org 0x0000

CLK	=	0x92 ; 0x92 == P1_2
MOSI	=	0x95 ; 0x95 == P1_5
MISO	=	0x94 ; 0x94 == P1_4
CS	=	0x93 ; 0x93 == P1_3
RESET  =   0x96 ; P1_6

    .area CSEG

; This is all designed to be interfaced with C code compiled by SDCC using SPIGeneric.h
; Therefore; the functions that take an argument expect the argument to be in DPL, and the functions that return a value place the value in DPL before returning

__spigeneric_frame_start:
    SETB RESET ; This should always be high, but just in case it somehow isn't, this is probably the cleanest place to set it
    CLR CLK
    CLR MOSI
    SETB MISO
    CLR CS
    RET

__spigeneric_frame_end:
    SETB    CS
    SETB    CLK
    SETB    MOSI
    SETB    MISO
    RET

__spigeneric_spit_bit:
	MOV	A,DPL
	ADD	A,#0xFF
	MOV	MOSI,C
	SETB	CLK
	JNB	MISO,MAKELOW
	MOV	DPL,#0x01
	SJMP	DONE
    MAKELOW:
        MOV	DPL,#0x00
    DONE:
        CLR	CLK
	RET

__spigeneric_spit_byte:
    MOV R7,DPL ;R7 stores the byte we are clocking out
	MOV R6,#0x00 ;R6 stores the byte we are returning
	MOV R5,#0x08 ;R5 is our loop counter since we gotta go through the loop eight times regardless of the contents of the byte

    TEST:
        CJNE	R5,#0x00,LOOPTY
        SJMP    ALLDONE
    LOOPTY:
        DEC R5
        MOV	A,#0x80
        ANL	A,R7
        MOV	DPL,A
        ACALL	__spigeneric_spit_bit
        MOV A,R6
        RL  A
        ORL	A,DPL
        MOV	R6,A
        MOV A,R7
        RL  A
        MOV	R7,A
        SJMP	TEST
    ALLDONE:
        MOV DPL,R6
        RET

__spigeneric_reset:
    CLR RESET
    NOP
    NOP
    SETB RESET