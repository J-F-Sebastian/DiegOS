include ../../../build/makefiles/makefile.master

OBJS = udivsi3.o umodsi3.o divsi3.o modsi3.o lshrdi3.o\
       ashrdi3.o ashldi3.o clzsi2.o clzdi2.o ctzsi2.o\
       lcmp.o ulcmp.o ldivmod.o uldivmod.o

CDEFS += -D__ARM_EABI__ -D__ARM_ARCH=5 -D__ARM_ARCH_ISA_ARM -D__ARM_ARCH_ISA_THUMB=1 -DL_dvmd_tls -D__TARGET_FEATURE_DSP

all:
	$(CC) $(CFLAGS) $(CDEFS) -DL_udivsi3 -c lib1funcs.S -o udivsi3.o 
	$(CC) $(CFLAGS) $(CDEFS) -DL_umodsi3 -c lib1funcs.S -o umodsi3.o 
	$(CC) $(CFLAGS) $(CDEFS) -DL_divsi3 -c lib1funcs.S -o divsi3.o 
	$(CC) $(CFLAGS) $(CDEFS) -DL_modsi3 -c lib1funcs.S -o modsi3.o 
	$(CC) $(CFLAGS) $(CDEFS) -DL_lshrdi3 -c lib1funcs.S -o lshrdi3.o
	$(CC) $(CFLAGS) $(CDEFS) -DL_ashrdi3 -c lib1funcs.S -o ashrdi3.o
	$(CC) $(CFLAGS) $(CDEFS) -DL_ashldi3 -c lib1funcs.S -o ashldi3.o
	$(CC) $(CFLAGS) $(CDEFS) -DL_clzsi2 -c lib1funcs.S -o clzsi2.o
	$(CC) $(CFLAGS) $(CDEFS) -DL_clzdi2 -c lib1funcs.S -o clzdi2.o
	$(CC) $(CFLAGS) $(CDEFS) -DL_ctzsi2 -c lib1funcs.S -o ctzsi2.o
	$(CC) $(CFLAGS) $(CDEFS) -DL_aeabi_lcmp -c lib1funcs.S -o lcmp.o
	$(CC) $(CFLAGS) $(CDEFS) -DL_aeabi_ulcmp -c lib1funcs.S -o ulcmp.o
	$(CC) $(CFLAGS) $(CDEFS) -DL_aeabi_ldivmod -c lib1funcs.S -o ldivmod.o
	$(CC) $(CFLAGS) $(CDEFS) -DL_aeabi_uldivmod -c lib1funcs.S -o uldivmod.o
	$(AR) $(CPU).a $(OBJS)

clean:
	rm -rf $(OBJS) *.a
