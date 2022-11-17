include ../../build/makefiles/makefile.master

OBJS = null.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))

CFLAGS+= -I../../include  -I../../include/libs

all: objdirs $(OBJSO)

clean:
	rm -rf $(OBJSO) *.a
