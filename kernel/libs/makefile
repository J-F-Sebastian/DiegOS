include $(WSROOT)/build/makefiles/makefile.master

OBJS =  

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))
 
all: objdirs $(OBJSO)
	cd pci && make all
	cd unistd && make all
	cd time && make all
	cd iomemory && make all
	$(AR) $(OBJPREFIX)_libkernel.a $(OBJSO)
	
clean:
	cd pci && make clean
	cd unistd && make clean
	cd time && make clean
	cd iomemory && make clean
	rm -rf $(OBJSO) *.a
