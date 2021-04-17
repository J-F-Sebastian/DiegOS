include ../../../build/makefiles/makefile.master

OBJS = sprintf.o snprintf.o vsnprintf.o printf.o fprintf.o\
	vprintf.o vsprintf.o vfprintf.o\
	doprnt.o icompute.o data.o putc.o fputc.o fflush.o\
	fopen.o fdopen.o freopen.o fclose.o remove.o rename.o\
	tmpfile.o setbuf.o setvbuf.o feof.o ferror.o clearerr.o\
	fputs.o doscan.o getc.o fgetc.o gets.o fgets.o\
	fscanf.o scanf.o sscanf.o vscanf.o vfscanf.o vsscanf.o\
	ungetc.o fileno.o writebuffer.o readbuffer.o\
	ftell.o fseek.o fgetpos.o fsetpos.o fread.o\
	fwrite.o perror.o puts.o ecvt.o setbuffer.o\
	setlinebuf.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))
 
CFLAGS+= -I../../../include 

all: objdirs $(OBJSO)
	$(AR) ../$(OBJPREFIX)_libc.a $(OBJSO)
	
clean:
	rm -rf $(OBJSO) *.a
