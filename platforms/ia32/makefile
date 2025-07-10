include $(WSROOT)/build/makefiles/makefile.master

OBJS = processor_init.o cpuid.o switch_context.o setup_context.o\
	interrupts.o ints.o ports.o hw_interrupts.o sw_interrupts.o\
    load_context.o exceptions.o delay.o fp_sse_handlers.o\
    init_ts.o init_fp.o init_simd.o msr.o apic.o mtrr.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))
 
CFLAGS+= -I../../libs

all: objdirs $(OBJSO)
	$(AR) $(CPU).a $(OBJSO)
	
clean: objclean
	rm -rf $(OBJSO) *.a
