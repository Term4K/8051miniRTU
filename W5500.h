#ifndef W5500_H

#include <8051.h>
#include "SPIGeneric.h"

#define W5500_H

#define SOCKET0 0
#define SOCKET1 1
#define SOCKET2 2
#define SOCKET3 3
#define SOCKET4 4
#define SOCKET5 5
#define SOCKET6 6
#define SOCKET7 7

#define SOCKETN_MODE 0x0000
#define SOCKETN_COMMAND 0x0001
#define SOCKETN_STATUS 0x0003
#define SOCKETN_SOURCEPORT 0x0004
#define SOCKETN_DEST_IP 0x000C
#define SOCKETN_DEST_PORT 0x010

#define SOCK_CLOSED 0x00
#define SOCK_INIT 0x13
#define SOCK_LISTEN 0x14
#define SOCK_ESTABLISHED 0x17
#define SOCK_CLOSE_WAIT 0x1C
#define SOCK_UDP 0x22
#define SOCK_MACRAW 0x42

#define PROTOCOL_TCP 0x01
#define PROTOCOL_UDP 0x02

#define W5500_BUFFER_SIZE 32

extern unsigned char _w5500_input_buffer[W5500_BUFFER_SIZE + 1];

// These are friendly defines because as real functions they would literally just be a wrapper for _w5500_write and _w5500_read, so this is faster and generates slightly less overhead

#define _w5500_set_gateway(gateway) _w5500_write(0x0001, 0x00, gateway, 4)
#define _w5500_get_gateway(gateway) _w5500_read(0x0001, 0x00, gateway, 4)

#define _w5500_set_subnet(subnet) _w5500_write(0x0005, 0x00, subnet, 4)
#define _w5500_get_subnet(subnet) _w5500_read(0x0005, 0x00, subnet, 4)

#define _w5500_set_mac(mac) _w5500_write(0x0009, 0x00, mac, 6)
#define _w5500_get_mac(mac) _w5500_read(0x0009, 0x00, mac, 6)

#define _w5500_set_ip(ip) _w5500_write(0x000F, 0x00, ip, 4);
#define _w5500_get_ip(ip) _w5500_read(0x000F, 0x00, ip, 4)

#define _w5500_set_phyconfig(phycfg) _w5500_write(0x002E, 0x00, 0x00, phycfg, 1)
unsigned char _w5500_get_phyconfig(); // This is an real function because we don't want to take an array as an argument to this 'friendly' interface when we can simply return an unsigned char instead

void _w5500_set_sourceport(unsigned char socket, unsigned int port);
void _w5500_set_remoteport(unsigned char socket, unsigned int port);
void _w5500_set_destinationip(unsigned char socket, unsigned char* ip);
void _w5500_writeto(unsigned char socket, const unsigned char* data, unsigned char length);
unsigned char* _w5500_readline(unsigned char socket);
unsigned int _w5500_bytesinbuffer(unsigned char socket);
unsigned char _w5500_get_status(unsigned char socket);
void _w5500_send(unsigned char socket);
void _w5500_set_udp(unsigned char socket);
void _w5500_set_tcp(unsigned char socket);
void _w5500_open(unsigned char socket);
void _w5500_close(unsigned char socket);
void _w5500_discon(unsigned char socket);
void _w5500_tcptick(unsigned char socket);
void _w5500_listen(unsigned char socket);
void _w5500_reset();
unsigned char _w5500_readchar(unsigned char socket);

// data must be an array at least as long as length, though these functions should not be called directly
void _w5500_read(unsigned int oa, unsigned char bsb, unsigned char* data, unsigned int length);
// data must be an array at least as long as length, though these functions should not be called directly
void _w5500_write(unsigned int oa, unsigned char bsb, const unsigned char* data, unsigned int length);

unsigned char _w5500_get_version();

#endif


