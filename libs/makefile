include $(WSROOT)/build/makefiles/makefile.master

OBJS = list.o queue.o stack.o chunks.o hash_list.o fnv.o pakman.o \
	red_black_tree.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))
 
all: objdirs $(OBJSO)
	cd libc && make all
	$(AR) $(OBJPREFIX)_diegos_lib.a $(OBJSO)
	
clean:
	cd libc && make clean
	rm -rf $(OBJSO) *.a
