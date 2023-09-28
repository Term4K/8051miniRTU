#ifndef SPIGENERIC_H

#include <8051.h>

#define SPIGENERIC_H

/* Output a byte using SPI, since SPI is theoretically full duplex this also returns the byte that came in over MISO.
   Reads can be performed by using this command with 0x00 as the argument.
   This function is for driver use and not to be called directly, generally speaking.
*/
unsigned char _spigeneric_spit_byte(unsigned char b);
void _spigeneric_frame_start(); // Initializes MOSI, MISO, SCLK, and CS. Always call this before _spigeneric_spit_byte
void _spigeneric_frame_end();  // Reset MOSI, MISO, SCLK, and pulls CS high. Every call of _spigeneric_frame_start should be matched with a subsequent call to this function
void _spigeneric_reset(); // This only works if there is a reset pin tied to the MC. Moreover, since it uses NOPs for timing, it is dependent on crystal/speed

#endif

