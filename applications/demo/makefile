include $(WSROOT)/build/makefiles/makefile.master

OBJS = demo_console.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))
 
CFLAGS+= -I$(WSROOT)/networking/include

all: objdirs $(OBJSO)
	$(AR) $(OBJPREFIX)_app.a $(OBJSO)
	
clean:
	rm -rf $(OBJSO) *.a
