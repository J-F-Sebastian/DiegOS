include $(WSROOT)/build/makefiles/makefile.master

OBJS = errno.o 

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))
 
all: objdirs $(OBJSO)
	cd ansi && make all
	cd stdio && make all
	cd quad && make all
	cd math && make all
	cd gnu/ieee_float && make all
	cd gnu/libgcc && make all
	$(AR) $(OBJPREFIX)_libc.a $(OBJSO)
	
clean:
	cd ansi && make clean
	cd stdio && make clean
	cd quad && make clean
	cd math && make clean
	cd gnu/ieee_float && make clean 
	cd gnu/libgcc && make clean
	rm -rf $(OBJSO) *.a
