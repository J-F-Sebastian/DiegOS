include ../../../../build/makefiles/makefile.master

OBJS = udivmoddi4.o udivmodti4.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))

CFLAGS+= -I../../../../include

all: objdirs $(OBJSO)
	$(AR) ../../$(OBJPREFIX)_libc.a $(OBJSO)

clean:
	rm -rf $(OBJSO) *.a
