include $(WSROOT)/build/makefiles/makefile.master

OBJS = threads.o scheduler.o mutex.o kernel.o idle_thread.o \
	kprintf.o clock.o fail_safe.o kernel_dump.o \
    events.o alarms.o barriers.o spinlocks.o io_waits.o \
    devices.o drivers.o kputb.o delays.o poll.o \
	net_interfaces.o timers.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))

OBJSTART = diegos_init.o

OBJSTARTO = $(addprefix $(OBJPREFIX)/, $(OBJSTART))

all: objdirs $(OBJSO) $(OBJSTARTO)
	cd libs && make all
	$(AR) $(OBJPREFIX)_kernel.a $(OBJSO)
	$(AR) $(OBJPREFIX)_diegos.a $(OBJSTARTO)

	
clean:
	cd libs && make clean
	rm -rf $(OBJSO) $(OBJSTARTO) *.a
