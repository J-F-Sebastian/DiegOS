include ../../build/makefiles/makefile.master

OBJS = processor_init.o cpuid.o switch_context.o setup_context.o\
	interrupts.o ints.o ports.o hw_interrupts.o sw_interrupts.o\
    load_context.o exceptions.o delay.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))
 
CFLAGS+= -I../../include -I../../libs -I../pc

all: $(OBJS)
	$(AR) $(CPU).a $(OBJS)
	
clean:
	rm -rf $(OBJS) *.a
