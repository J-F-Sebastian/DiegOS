include ../../build/makefiles/makefile.master

OBJS =  

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))
 
CFLAGS+= -I../../include 

all: objdirs $(OBJSO)
	cd pci && make all
	cd unistd && make all
	cd time && make all
	$(AR) $(OBJPREFIX)_libkernel.a $(OBJSO)
	
clean:
	cd pci && make clean
	cd unistd && make clean
	cd time && make clean
	rm -rf $(OBJSO) *.a
