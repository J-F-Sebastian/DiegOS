include $(WSROOT)/build/makefiles/makefile.master

all:
	cd UART && $(MAKE) all
	cd TIMER && $(MAKE) all
	$(AR) $(OBJPREFIX)_drivers.a UART/$(OBJPREFIX)/wb_uart.o TIMER/$(OBJPREFIX)/N9H26_TIMER.o

clean:
	cd UART && $(MAKE) clean
	cd TIMER && $(MAKE) clean
	rm -rf *.a
