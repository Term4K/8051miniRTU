#ifndef SERIAL_H

#include <8051.h>

#define SERIAL_H

#define CRYSTAL_FREQ 11.0592
#define BAUD_RATE 9600

#define MAGIC_TH1 ((256) - CRYSTAL_FREQ * 1000 * 1000 / 6 / 32 / BAUD_RATE)

#define BUFFER_SIZE 32

extern unsigned char input_buffer[BUFFER_SIZE + 1];

extern volatile unsigned char bufferlo_bill[BUFFER_SIZE + 1];
extern volatile unsigned char bufferlo_bill_counter;

void _serial_begin(); // Call this once before doing serial things to set the registers appropriately, this uses timer 1
void _serial_putchar (unsigned char c); // Output a single character over serial
void _serial_putstr(unsigned char* s); // Output a properly formatted c-string over serial
unsigned char _serial_getchar(); // Reads a single byte/character from serial and returns it
unsigned char* _serial_emptybuffer();

void _serial_putbyte_dec(unsigned char b);
void _serial_putbyte_hex(unsigned char b);

#endif
