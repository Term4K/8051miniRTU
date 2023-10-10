# RTU SIMULATION
This is a simple RTU simulation using an 8051 architecture board. This will accept both UDP and TCP packets. It will take any in the proper format (:<message>) and return a message in the propper return format with the message being in all caps (:[MESSAGE]). The switch is used to switch between TCP and UDP. The user can also use the serial interface to change the IP, Subnet, Gateway, and MAC Address. All traffic is routed through the W5500 chip and uses SPI to communicate between the STC89 and itself.

# PARTS
```
STC89/STC12 DEMO BOARD
WIZnet W5500
1 SWITCH
3 LEDs
```

# MAKEFILE
The Makefile will build all this for you. You need to have SDCC, SDAS and Python. Python is used for the STCGAL to load the HEX onto the chip.  

