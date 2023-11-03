include ../../../build/makefiles/makefile.master

OBJS = udivsi3.o umodsi3.o divsi3.o modsi3.o lshrdi3.o\
       ashrdi3.o ashldi3.o muldf3.o muldivdf3.o addsubdf3.o\
       clzsi2.o clzdi2.o ctzsi2.o truncdfsf2.o cmpdf2.o\
       lcmp.o ulcmp.o ldivmod.o uldivmod.o fixdfsi.o fixunsdfsi.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))

CDEFS += -D__ARM_EABI__ -D__ARM_ARCH=5 -D__ARM_ARCH_ISA_ARM -D__ARM_ARCH_ISA_THUMB=1 -DL_dvmd_tls -D__TARGET_FEATURE_DSP
CFLAGS+= -I../../../include

all: objdirs
	$(CC) $(CFLAGS) $(CDEFS) -DL_udivsi3 -c lib1funcs.S -o $(OBJPREFIX)/udivsi3.o
	$(CC) $(CFLAGS) $(CDEFS) -DL_umodsi3 -c lib1funcs.S -o $(OBJPREFIX)/umodsi3.o
	$(CC) $(CFLAGS) $(CDEFS) -DL_divsi3 -c lib1funcs.S -o $(OBJPREFIX)/divsi3.o
	$(CC) $(CFLAGS) $(CDEFS) -DL_modsi3 -c lib1funcs.S -o $(OBJPREFIX)/modsi3.o
	$(CC) $(CFLAGS) $(CDEFS) -DL_lshrdi3 -c lib1funcs.S -o $(OBJPREFIX)/lshrdi3.o
	$(CC) $(CFLAGS) $(CDEFS) -DL_ashrdi3 -c lib1funcs.S -o $(OBJPREFIX)/ashrdi3.o
	$(CC) $(CFLAGS) $(CDEFS) -DL_ashldi3 -c lib1funcs.S -o $(OBJPREFIX)/ashldi3.o
	$(CC) $(CFLAGS) $(CDEFS) -DL_arm_muldf3 -c lib1funcs.S -o $(OBJPREFIX)/muldf3.o
	$(CC) $(CFLAGS) $(CDEFS) -DL_arm_muldivdf3 -c lib1funcs.S -o $(OBJPREFIX)/muldivdf3.o
	$(CC) $(CFLAGS) $(CDEFS) -DL_arm_addsubdf3 -c lib1funcs.S -o $(OBJPREFIX)/addsubdf3.o
	$(CC) $(CFLAGS) $(CDEFS) -DL_clzsi2 -c lib1funcs.S -o $(OBJPREFIX)/clzsi2.o
	$(CC) $(CFLAGS) $(CDEFS) -DL_clzdi2 -c lib1funcs.S -o $(OBJPREFIX)/clzdi2.o
	$(CC) $(CFLAGS) $(CDEFS) -DL_ctzsi2 -c lib1funcs.S -o $(OBJPREFIX)/ctzsi2.o
	$(CC) $(CFLAGS) $(CDEFS) -DL_arm_cmpdf2 -c lib1funcs.S -o $(OBJPREFIX)/cmpdf2.o
	$(CC) $(CFLAGS) $(CDEFS) -DL_arm_truncdfsf2 -c lib1funcs.S -o $(OBJPREFIX)/truncdfsf2.o
	$(CC) $(CFLAGS) $(CDEFS) -DL_aeabi_lcmp -c lib1funcs.S -o $(OBJPREFIX)/lcmp.o
	$(CC) $(CFLAGS) $(CDEFS) -DL_aeabi_ulcmp -c lib1funcs.S -o $(OBJPREFIX)/ulcmp.o
	$(CC) $(CFLAGS) $(CDEFS) -DL_aeabi_ldivmod -c lib1funcs.S -o $(OBJPREFIX)/ldivmod.o
	$(CC) $(CFLAGS) $(CDEFS) -DL_aeabi_uldivmod -c lib1funcs.S -o $(OBJPREFIX)/uldivmod.o
	$(CC) $(CFLAGS) $(CDEFS) -DL_arm_fixdfsi -c lib1funcs.S -o $(OBJPREFIX)/fixdfsi.o
	$(CC) $(CFLAGS) $(CDEFS) -DL_arm_fixunsdfsi -c lib1funcs.S -o $(OBJPREFIX)/fixunsdfsi.o
	$(AR) $(OBJPREFIX).a $(OBJSO)

clean:
	rm -rf $(OBJSO) $(OBJPREFIX).a
