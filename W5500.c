#include "W5500.h"

unsigned char small_buffer[2];
unsigned char _w5500_input_buffer[W5500_BUFFER_SIZE + 1];

void _w5500_open(unsigned char socket) {
    small_buffer[0] = 0x1;
    _w5500_write(SOCKETN_COMMAND, (socket * 4) + 1, small_buffer, 1);
}

void _w5500_reset() {
    small_buffer[0] = 0x80;
    _w5500_write(0x00, 0x00, small_buffer, 1);
}

void _w5500_close(unsigned char socket) {
    small_buffer[0] = 0x10;
    _w5500_write(SOCKETN_COMMAND, (socket * 4) + 1, small_buffer, 1);
}

void _w5500_discon(unsigned char socket) {
    small_buffer[0] = 0x08;
    _w5500_write(SOCKETN_COMMAND, (socket * 4) + 1, small_buffer, 1);
}

void _w5500_set_udp(unsigned char socket) {
    _w5500_read(0x00, (socket * 4) + 1, small_buffer, 1);
    small_buffer[0] &= 0xF0;
    small_buffer[0] |= PROTOCOL_UDP;
    _w5500_write(0x00, (socket * 4) + 1, small_buffer, 1);
}

void _w5500_set_tcp(unsigned char socket) {
    _w5500_read(0x00, (socket * 4) + 1, small_buffer, 1);
    small_buffer[0] &= 0xF0;
    small_buffer[0] |= PROTOCOL_TCP;
    _w5500_write(0x00, (socket * 4) + 1, small_buffer, 1);
}

unsigned char _w5500_get_status(unsigned char socket) {
    _w5500_read(0x03, (socket * 4) + 1, small_buffer, 1);
    return small_buffer[0];
}

void _w5500_listen(unsigned char socket) {
    small_buffer[0] = 0x02;
    _w5500_write(SOCKETN_COMMAND, (socket * 4) + 1, small_buffer, 1);
}

void _w5500_tcptick(unsigned char socket) {
    small_buffer[0] = 0x22;
    _w5500_write(SOCKETN_COMMAND, (socket * 4) + 1, small_buffer, 1);
}

void _w5500_set_sourceport(unsigned char socket, unsigned int port) {
    small_buffer[0] = port >> 8;
    small_buffer[1] = port & 0xFF;
    _w5500_write(SOCKETN_SOURCEPORT, (socket * 4) + 1, small_buffer, 2);
}

void _w5500_set_remoteport(unsigned char socket, unsigned int port) {
    small_buffer[0] = port >> 8;
    small_buffer[1] = port & 0xFF;
    _w5500_write(SOCKETN_DEST_PORT, (socket * 4) + 1, small_buffer, 2);
}

void _w5500_set_destinationip(unsigned char socket, unsigned char* ip) {
    _w5500_write(SOCKETN_DEST_IP, (socket * 4) + 1, ip, 4);
}


void _w5500_writeto(unsigned char socket, const unsigned char* data, unsigned char length) {
    unsigned int easy_math = 0; // Purpose of easy_math is to easily add the amount of data we wrote to the wiznet buffer pointer, using an int is easier than adding to an unsigned char with manual carry
    _w5500_read(0x24, (socket * 4) + 1, small_buffer, 2);
    easy_math = (small_buffer[0] << 8) | small_buffer[1];
    _w5500_write(easy_math, (socket * 4) + 2, data, length);
    easy_math += length;
    small_buffer[0] = easy_math >> 8;
    small_buffer[1] = easy_math & 0xFF;
    _w5500_write(0x24, (socket * 4) + 1, small_buffer, 2);
}

unsigned int _w5500_bytesinbuffer(unsigned char socket) {
    _w5500_read(0x26, (socket * 4) + 1, small_buffer, 2);
    return (small_buffer[0] << 8) | small_buffer[1];
}

/* General outline of this function; zero out the 8051 input buffer, discover how many bytes are in the wiznet's input buffer.
   Read the wiznet input buffer pointer, and starting there read one character at a time until we find a LF character.
   Once we find a LF character, set a flag so that if the next character we find is a CR, we break the loop. LF CR ends a line.
   Slap a zero at the end of the buffer to make it a string, update the wiznet so the read pointer points to the next unread data.
*/

unsigned char* _w5500_readline(unsigned char socket) {
    unsigned int read_size = 0;
    unsigned int starting_addr = 0;
    unsigned char prime = 0;
    unsigned char x = 0;

    for (read_size = 0; read_size < W5500_BUFFER_SIZE; ++read_size) {
        _w5500_input_buffer[read_size] = 0;
    }
    read_size = 0;
    read_size = _w5500_bytesinbuffer(socket);
    _w5500_read(0x28, (socket * 4) + 1, small_buffer, 2);
    starting_addr = (small_buffer[0] << 8) | small_buffer[1];

    for (x = 0; x < read_size && x < W5500_BUFFER_SIZE; ++x) {
        _w5500_read(starting_addr, (socket * 4) + 3, (_w5500_input_buffer + x), 1);
        if (prime == 0 && *(_w5500_input_buffer + x) == 0x0D) prime = 1; else if (prime == 1 && *(_w5500_input_buffer + x) == 0x0A) break; else prime = 0;
        ++starting_addr;
    }
    _w5500_input_buffer[x + 1] = 0;
    small_buffer[0] = starting_addr >> 8;
    small_buffer[1] = starting_addr & 0xFF;
    _w5500_write(0x28, (socket  * 4) + 1, small_buffer, 2);
    small_buffer[0] = 0x40;
    _w5500_write(SOCKETN_COMMAND, (socket * 4) + 1, small_buffer, 1);
    return _w5500_input_buffer;
}

/* General outline of this function; zero out the 8051 input buffer, discover how many bytes are in the wiznet's input buffer.
   If we can hold the entire wiznet input buffer in 8051 input buffer, read the entire thing in, otherwise read as much as we can hold.
   Slap a zero at the end of the buffer to make it a string, and update the wiznet so the read pointer points to the next unread data.
*/

// Debugging function
unsigned char _w5500_readchar(unsigned char socket) {
    unsigned char val = 0;
    unsigned int starting_addr = 0;
    _w5500_read(0x28, (socket * 4) + 1, small_buffer, 2);
    starting_addr = (small_buffer[0] << 8) | small_buffer[1];
    if (_w5500_bytesinbuffer(socket) == 0) return 0;
    _w5500_read(starting_addr, (socket * 4) + 3, &val, 1);
    starting_addr++;
    small_buffer[0] = starting_addr >> 8;
    small_buffer[1] = starting_addr & 0xFF;
    _w5500_write(0x28, (socket  * 4) + 1, small_buffer, 2);
    small_buffer[0] = 0x40;
    _w5500_write(SOCKETN_COMMAND, (socket * 4) + 1, small_buffer, 1);
    return val;
}

void _w5500_send(unsigned char socket) {
    small_buffer[0] = 0x20;
    _w5500_write(SOCKETN_COMMAND, (socket * 4) + 1, small_buffer, 1);
}

unsigned char _w5500_get_version() {
    _w5500_read(0x0039, 0x00, small_buffer, 1);
    return small_buffer[0];
}

unsigned char _w5500_get_phyconfig() {
    _w5500_read(0x002E, 0x00, small_buffer, 1);
    return small_buffer[0];
}

void _w5500_read(unsigned int oa, unsigned char bsb, unsigned char* data, unsigned int length) {
    unsigned char oa_high = oa >> 8;
    unsigned char oa_low = oa & 0xFF;
    unsigned char true_bsb = ((bsb << 3) | (0 << 2)); // | om, this was removed because om always == 0 in my use cases

    _spigeneric_frame_start();

    _spigeneric_spit_byte(oa_high);
    _spigeneric_spit_byte(oa_low);
    _spigeneric_spit_byte(true_bsb);
    while (length--)    *(data++) = _spigeneric_spit_byte(0x00);

    _spigeneric_frame_end();
}

void _w5500_write(unsigned int oa, unsigned char bsb, const unsigned char* data, unsigned int length) {
    unsigned char oa_high = oa >> 8;
    unsigned char oa_low = oa & 0xFF;
    unsigned char true_bsb = ((bsb << 3) | (1 << 2)); // | om, this was removed because om always == 0 in my use cases

    _spigeneric_frame_start();

    _spigeneric_spit_byte(oa_high);
    _spigeneric_spit_byte(oa_low);
    _spigeneric_spit_byte(true_bsb);
    while (length--) _spigeneric_spit_byte(*(data++));

    _spigeneric_frame_end();
}
