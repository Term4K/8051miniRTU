#include "Serial.h"

unsigned char input_buffer[BUFFER_SIZE + 1];
volatile unsigned char bufferlo_bill[BUFFER_SIZE + 1];
volatile unsigned char bufferlo_bill_counter = 0;

void _serial_begin() {
    __asm;
    MOV TMOD,#0x20
    MOV SCON,#0xD8
	;MOV SCON,#0x50
	__endasm;
	TH1 = (unsigned char) MAGIC_TH1;
    __asm;
    MOV A,TCON
    ORL A,#0x40
    MOV TCON,A
    MOV A,SCON
    ORL A,#0x02
    MOV SCON,A
    MOV IE,#0x90
    __endasm;
}

void _serial_putchar(unsigned char c) {
    while (!TI);
    SBUF = c;
    TI = 0;
    while (!TI);
}

void _serial_putstr(unsigned char* s) {
    while (*s) _serial_putchar(*(s++));
}

void _serial_putbyte_hex(unsigned char b) {
    _serial_putchar((b >> 4) > 9 ? (b >> 4) + 0x37 : (b >> 4) + 0x30);
    _serial_putchar((b & 0xF) > 9 ? (b & 0xF) + 0x37 : (b & 0xF) + 0x30);
}

void _serial_putbyte_dec(unsigned char b) {
    if (b / 100) _serial_putchar((b / 100) + 0x30);
    if ((b % 100) / 10) _serial_putchar(((b % 100) / 10) + 0x30);
    _serial_putchar((b % 10) + 0x30);
}

unsigned char _serial_getchar() {
    unsigned char read = 0;
    while (!RI);
    read = SBUF;
    RI = 0;
    return read;
}

unsigned char* _serial_emptybuffer() {
    unsigned char count = 0;
    __asm;
    CLR EA
    __endasm;

    while (bufferlo_bill[count] != 0) {
        input_buffer[count] = bufferlo_bill[count];
        ++count;
    }

    input_buffer[count] = bufferlo_bill[count];
    bufferlo_bill_counter = 0;
    bufferlo_bill[0] = 0;

    __asm;
    SETB    EA
    __endasm;
    return input_buffer;
}

void _serial_interrupt(void) __interrupt (4) {
    if (RI) {
        __asm;
        CLR EA
        CLR RI
        __endasm;
        bufferlo_bill[bufferlo_bill_counter] = SBUF;
        ++bufferlo_bill_counter;
        bufferlo_bill[bufferlo_bill_counter] = 0;
        if (bufferlo_bill_counter > BUFFER_SIZE) bufferlo_bill_counter = 0;
        __asm;
        SETB    EA
        __endasm;
    }
}
