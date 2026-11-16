include $(WSROOT)/build/makefiles/makefile.master

OBJS = file_system.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))

all: objdirs $(OBJSO)
	$(AR) $(OBJPREFIX)_app.a $(OBJSO)

clean:
	rm -rf $(OBJSO) *.a