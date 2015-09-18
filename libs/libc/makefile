include ../../build/makefiles/makefile.master

OBJS = errno.o 

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))
 
CFLAGS+= -I../../include 

all: objdirs $(OBJSO)
	cd ansi && make all
	cd stdio && make all
	cd quad && make all
	$(AR) $(OBJPREFIX)_libc.a $(OBJSO)
	
clean:
	cd ansi && make clean
	cd stdio && make clean
	cd quad && make clean
	rm -rf $(OBJSO) *.a
