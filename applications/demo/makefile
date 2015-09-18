include ../../build/makefiles/makefile.master

OBJS = demo_console.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))
 
CFLAGS+= -I../../include -I../../include/libs -I../../networking/include

all: objdirs $(OBJSO)
	$(AR) $(OBJPREFIX)_app.a $(OBJSO)
	
clean:
	rm -rf $(OBJSO) *.a
