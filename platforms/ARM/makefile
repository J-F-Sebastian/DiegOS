include $(WSROOT)/build/makefiles/makefile.master

OBJS = processor.o mmu.o interrupts.o startup.o \
       load_context.o switch_context.o setup_context.o cleanup_context.o \
       power_save.o delay.o ARM_irq.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))
 
all: objdirs $(OBJSO)
	$(AR) $(OBJPREFIX)_ARM.a $(OBJSO)
	
clean: objclean
