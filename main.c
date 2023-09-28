#include <8051.h>
#include "Serial.h"
#include "SPIGeneric.h"
#include "W5500.h"


/* All of these are constant because they are just used to set up the wiznet configuration initially. Wiznet itself will store the 'real' values, for example if the IP webpage is used to change the IP.
   When the values are dumped over serial, they are read from the wiznet and temporarily stored in op.
*/
volatile unsigned char gateway[4] = {192, 168, 16, 1};
volatile unsigned char subnet[4] = {255, 255, 255, 0};
volatile unsigned char ip[4] = {192, 168, 16, 222};
volatile unsigned char mac[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
volatile unsigned char dest_ip[4] = {192, 168, 16, 187};
void _serial_putbyte_hex(unsigned char b);
enum wizMode {UDP, TCP};
enum wizMode mode = UDP; //var for settings mode of wiznet, changed with serial
unsigned char rtuAdd = '3';

#ifndef NULL
#define NULL ((void *)0)
#endif
// String pointer
unsigned char* input;
unsigned char* serialInput;

void _serial_interrupt(void) __interrupt (4);

unsigned char strlen(const unsigned char* str) {
    unsigned char ret = 0;
    while (*(str++)) ++ret;
    return ret;
}

unsigned char strcmp(unsigned char* first, unsigned char* second) {
    while (*first && *second) if (*(first++) != *(second++)) return 0;
    return 1;
}

void magic_delay() {
    for (unsigned char x = 0; x < 64; ++x) {
        for (unsigned char y = 0; y < 64; ++y) {
            __asm;
            nop
            __endasm;
        }
    }
}

unsigned char hexCharToInt(char c) {
    return c <= '9' ? c - '0' : c <= 'F' ? c - 'A' + 10 : c - 'a' + 10;
}

// Parse MAC address from the input string and store it in the mac array
void parseMAC(const char* str) {
    // Check if the string starts with "MAC="
    if(str[0] == 'M' && str[1] == 'A' && str[2] == 'C' && str[3] == '=') {
        for(unsigned char i = 0; i < 6; i++) {
            mac[i] = (hexCharToInt(str[4 + i*2]) << 4) + hexCharToInt(str[5 + i*2]);
        }
    }
}

void extractAndAssign(char *arr)
{
    unsigned char *curr_address = NULL;
    unsigned char octet = 0;
    unsigned char value = 0;

    while (*arr)
    {
        if (arr[0] == 'I' && arr[1] == 'P' && arr[2] == '=')
        {
            curr_address = ip;
            arr += 3;
        }
        else if (arr[0] == 'S' && arr[1] == 'U' && arr[2] == 'B' && arr[3] == '=')
        {
            curr_address = subnet;
            arr += 4;
        }
        else if (arr[0] == 'G' && arr[1] == 'A' && arr[2] == 'T' && arr[3] == 'E' && arr[4] == '=')
        {
            curr_address = gateway;
            arr += 5;
        }
        value = 0;
        while (*arr >= '0' && *arr <= '9')
        {
            value = value * 10 + (*arr - '0');
            arr++;
        }

        if (curr_address && octet < 4)
        {
            curr_address[octet] = value;
            octet++;

            if (*arr == '.')
            {
                arr++;
                continue;
            }
            else
            {
                octet = 0; // Reset octet count for the next IP address
            }
        }
        arr++;
    }
}

void displayConfig()
{
    _serial_putstr("Current Config:\n\r");
    _serial_putstr("-RTU Address (0-9): ");
    _serial_putchar(rtuAdd); // assuming rtu_address is a number from 0-9
    _serial_putstr(" CHANGE RTU=\n\r");

    _serial_putstr("-Network Mode (UDP/TCP): ");
    _serial_putstr(mode == UDP ? "UDP" : "TCP");
    _serial_putstr(" CHANGE MODE=\n\r");

    _serial_putstr("-IP Address: ");
    for (unsigned char i = 0; i < 4; i++)
    {
        _serial_putbyte_dec(ip[i]);
        if (i != 3)
            _serial_putchar('.');
    }
    _serial_putstr(" CHANGE IP=\n\r");

    _serial_putstr("-Subnet Mask: ");
    for (unsigned char i = 0; i < 4; i++)
    {
        _serial_putbyte_dec(subnet[i]);
        if (i != 3)
            _serial_putchar('.');
    }
    _serial_putstr(" CHANGE SUB=\n\r");

    _serial_putstr("-Gateway: ");
    for (unsigned char i = 0; i < 4; i++)
    {
        _serial_putbyte_dec(gateway[i]);
        if (i != 3)
            _serial_putchar('.');
    }
    _serial_putstr(" CHANGE GATE=\n\r");

    _serial_putstr("-MAC Address: ");
    for (unsigned char i = 0; i < 6; i++)
    {
        _serial_putbyte_hex(mac[i]);
        if (i != 5)
            _serial_putstr(" ");
    }
    _serial_putstr(" CHANGE MAC=\n\r");
}

void setup() {
    _serial_begin();
    _spigeneric_reset(); // Hardware reset, I don't think this is necessary but I put it in just in case
    _w5500_set_gateway(gateway);
    _w5500_set_subnet(subnet);
    _w5500_set_ip(ip);
    _w5500_set_mac(mac);

    _w5500_set_destinationip(SOCKET0, dest_ip);
    _w5500_set_sourceport(SOCKET1, 8181);
    _w5500_set_sourceport(SOCKET0, 5001); // Source port is 80 because it doesn't really matter where we send from, but if we go webserver mode we will expect incoming connections on port 80
    _w5500_set_remoteport(SOCKET0, 59663); // Magic number that is only used once
    _w5500_set_udp(SOCKET0);
    _w5500_open(SOCKET0);
    P2_0 = 1;
    P2_1 = 1;
    P2_2 = 1;
    P2_4 = 0;
    P2_5 = 0;
}

void loop() {
    unsigned char read = 0;
    unsigned char suicide = 0;
    unsigned char recieved = 0;
    unsigned char inputBuffer[20];
    unsigned char serialBuffer[20];
    unsigned char serialIndex = 0;
    unsigned char inputIndex = 0;
    unsigned char recvChar;
    _w5500_close(SOCKET1);
begin_anew:
    //_serial_putstr("Stuck?1");
    suicide = 0;
    _w5500_discon(SOCKET1);
    //while (_w5500_get_status(SOCKET1) != SOCK_CLOSED) {_serial_putstr("Stuck?2"); continue;};
    _w5500_set_tcp(SOCKET1);
    _w5500_open(SOCKET1);
    _w5500_listen(SOCKET1);
    while (1) {
        serialInput = _serial_emptybuffer();
        unsigned char inputLength = strlen(serialInput);
        if (inputLength) {
            if(strcmp(serialInput, "?"))
                displayConfig();
            else {
                if(strcmp(serialInput, "\r")){
                    if(serialBuffer[0]=='I' && serialBuffer[2]=='='){
                        extractAndAssign(serialBuffer);
                        _w5500_set_ip(ip);
                    } else if(serialBuffer[0]=='R' && serialBuffer[3]=='='){
                        rtuAdd = serialBuffer[4];
                    } else if(serialBuffer[0]=='M' || serialBuffer[4]=='='){
                        serialBuffer[serialIndex] = '\r';
                        if(serialBuffer[5]=='U'){
                            mode = UDP;
                            _serial_putstr("\n\rMODE UDP");
                        }else {
                            mode = TCP;
                            _serial_putstr("\n\rMODE TCP");
                        }
                    } else if(serialBuffer[0]=='G' && serialBuffer[4]=='='){
                        extractAndAssign(serialBuffer);
                        _w5500_set_gateway(gateway);
                    } else if(serialBuffer[0]=='S' && serialBuffer[3]=='='){
                        extractAndAssign(serialBuffer);
                        _w5500_set_subnet(subnet);
                    } else if(serialBuffer[0]=='M' && serialBuffer[3]=='='){
                        parseMAC(serialBuffer);
                        _w5500_set_mac(mac);
                    } else {
                        _serial_putstr("\n\rInvalid Command");
                    }

                    for(unsigned char i = 0; i< 20; i++){
                        serialBuffer[i] = '\0';
                    }
                    _serial_putstr("\n");
                    serialIndex=0;
                } else {
                    for (unsigned char charIndex = 0; charIndex < inputLength; charIndex++)
                    {
                        serialBuffer[serialIndex] = serialInput[charIndex];
                        serialIndex++;
                    }
                }
                _serial_putstr(serialInput);
            }
        }
        if(P2_4 == 1 && mode == UDP){
            mode = TCP;
            P2_2 = 0;
        } else if(P2_4 == 0 && mode == TCP){
            mode = UDP;
            P2_2 = 1;
        }
        read = 0;
        if(mode == UDP){
            while (_w5500_bytesinbuffer(SOCKET0)) {
                __asm;
                CLR EA
                __endasm;
                P2_0 = !P2_0;
                //_serial_putstr("Stuck?3");
                ++read;
                if (read < 9) {
                    _w5500_readchar(SOCKET0);
                    continue;
                }
                recvChar = _w5500_readchar(SOCKET0);
                //_serial_putcharspec(recvChar, read < 11 ? 1 : 0);
                if(recvChar >= 97 && recvChar <= 122)
                    inputBuffer[inputIndex] = recvChar-32;
                else if(recvChar >= 65 && recvChar <= 90)
                    inputBuffer[inputIndex] = recvChar;
                else 
                    inputBuffer[inputIndex] = recvChar;
                //_serial_putchar(inputBuffer[inputIndex]);
                inputIndex++;
                recieved = 1;
            }
            P2_0 = 1;
            if(recieved){
                P2_1 = !P2_1;
                if(inputBuffer[0] == ':' && inputBuffer[1] == '<' && inputBuffer[inputIndex-1] == '>' && inputBuffer[2] >= '0' && inputBuffer[2] <= '9'){
                    P2_1 = !P2_1;
                    inputBuffer[1] = '[';
                    inputBuffer[inputIndex-1] = ']';
                    if(inputBuffer[2] == rtuAdd){
                        P2_1 = !P2_1;
                        _w5500_writeto(SOCKET0, inputBuffer, inputIndex);
                        _w5500_send(SOCKET0);
                        P2_1 = !P2_1;
                    }
                } else {
                    unsigned char fail[13] = "Wrong Format";
                    _w5500_writeto(SOCKET0, fail, 12);
                    _w5500_send(SOCKET0);
                }
                for(unsigned char j = 0; j < 20; j++){
                    P2_1 = !P2_1;
                    inputBuffer[j] = '\0';
                }
                inputIndex = 0;
                recieved = 0;
            }
            P2_1 = 1;
            __asm;
            SETB EA;
            __endasm;
        }else if(mode == TCP){
            //_serial_putstr("Stuck?3");
            magic_delay();
            _w5500_tcptick(SOCKET1);
            if (_w5500_get_status(SOCKET1) != SOCK_ESTABLISHED) {
                goto begin_anew;
            }
            if (_w5500_bytesinbuffer(SOCKET1)){
                P2_0 = !P2_0;
                suicide = 0;
                while (_w5500_bytesinbuffer(SOCKET1)) {
                    P2_0 = !P2_0;
                    input = _w5500_readline(SOCKET1);
                    P2_0 = !P2_0;
                    //_serial_putstr(input);

                    for(unsigned char i = 0; i < strlen(input); i++){
                        P2_0 = !P2_0;
                        recvChar = input[i];
                        if(recvChar >= 97 && recvChar <= 122)
                            inputBuffer[inputIndex] = recvChar-32;
                        else if(recvChar >= 65 && recvChar <= 90)
                            inputBuffer[inputIndex] = recvChar;
                        else 
                            inputBuffer[inputIndex] = recvChar;
                        inputIndex++;
                        recieved = 1;
                    }
                    P2_0 = 1;
                    if(recieved){
                        if(inputBuffer[0] == ':' && inputBuffer[1] == '<' && inputBuffer[inputIndex-1] == '>' && inputBuffer[2] >= '0' && inputBuffer[2] <= '9'){
                            P2_1 = !P2_1;
                            inputBuffer[1] = '[';
                            inputBuffer[inputIndex-1] = ']';
                            if(inputBuffer[2] == rtuAdd){
                                P2_1 = !P2_1;
                                _w5500_writeto(SOCKET1, inputBuffer, inputIndex);
                                _w5500_send(SOCKET1);
                                P2_1 = !P2_1;
                            }
                        } else {
                            unsigned char fail[13] = "Wrong Format";
                            _w5500_writeto(SOCKET1, fail, 12);
                            _w5500_send(SOCKET1);
                        }
                        for(unsigned char j = 0; j < 20; j++){
                            P2_1 = !P2_1;
                            inputBuffer[j] = '\0';
                        }
                        inputIndex = 0;
                        recieved = 0;
                    }
                    magic_delay();
                    
                }
                P2_1 = 1;
                goto begin_anew;
            } else {
                ++suicide;
                if (suicide == 0xFF) {
                    goto begin_anew;
                }
            }
        } 
    }
}

void main(void) {
    setup();
    while(1) {
        loop();
    }
}
