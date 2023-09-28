CC=sdcc
CFLAGS=-mmcs51 --opt-code-size --model-medium
ASM=sdas8051
AFLAGS=-los
MAIN=main.c
AMAIN=SPIGeneric.asm

makeit: 
	@-rm -f *.lk
	@-rm -f *.lst
	@-rm -f *.map
	@-rm -f *.mem
	@-rm -f *.rel
	@-rm -f *.rst
	@-rm -f *.sym
	@for file in *.c ; do if [ $$file != $(MAIN) ]; then $(CC) $(CFLAGS) -c $$file; fi done
	$(ASM) $(AFLAGS) $(AMAIN)
	$(CC) $(CFLAGS) $(MAIN) *.rel

Release:
	@-rm -f *.lk
	@-rm -f *.lst
	@-rm -f *.map
	@-rm -f *.mem
	@-rm -f *.rel
	@-rm -f *.rst
	@-rm -f *.sym
	@for file in *.c ; do if [ $$file != $(MAIN) ]; then $(CC) $(CFLAGS) -c $$file; fi done
	$(ASM) $(AFLAGS) $(AMAIN)
	$(CC) $(CFLAGS) $(MAIN) *.rel

clean:
	@for file in *.asm ; do if [ $$file != $(AMAIN) ]; then rm $$file; fi done
	-rm *.ihx
	-rm *.lk
	-rm *.lst
	-rm *.map
	-rm *.mem
	-rm *.rel
	-rm *.rst
	-rm *.sym

load:
	stcgal -o cpu_6t_enabled=TRUE main.ihx
