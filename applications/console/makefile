include ../../build/makefiles/makefile.master

OBJS = console.o system_parser.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))
 
CFLAGS+= -I../../include -I../../include/libs 

all: objdirs $(OBJSO)
	$(AR) $(OBJPREFIX)_app.a $(OBJSO)
	
clean:
	rm -rf $(OBJSO) *.a
