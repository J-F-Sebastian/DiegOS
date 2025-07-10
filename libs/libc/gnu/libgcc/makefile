include $(WSROOT)/build/makefiles/makefile.master

OBJS = udivmoddi4.o udivmodti4.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))

all: objdirs $(OBJSO)
	$(AR) ../../$(OBJPREFIX)_libc.a $(OBJSO)

clean:
	rm -rf $(OBJSO) *.a
