include ../../../build/makefiles/makefile.master

OBJS = strcpy.o strncpy.o strcat.o strncat.o strcmp.o strncmp.o\
	strchr.o strrchr.o strspn.o strcspn.o strpbrk.o strstr.o\
	strlen.o strerror.o strtok.o strtok_r.o\
	memchr.o memcpy_$(CPU).o memmove.o memset.o memcmp.o\
	toupper.o tolower.o malloc.o calloc.o rand.o abort.o\
	exit.o system.o getenv.o bsearch.o qsort.o abs.o labs.o\
	div.o ldiv.o atoi.o atol.o atof.o strtol.o\
	isalnum.o isalpha.o iscntrl.o isdigit.o isgraph.o\
	islower.o isprint.o ispunct.o isspace.o isupper.o isxdigit.o\
	errlist.o getenv.o assert.o clock.o asctime.o difftime.o\
	time.o ctime.o localtime.o mktime.o tzset.o ansidata.o\
	ext_comp.o progname.o convtime.o gmtime.o

OBJSO = $(addprefix $(OBJPREFIX)/, $(OBJS))
 
CFLAGS+= -I../../../include 

all: objdirs $(OBJSO)
	$(AR) ../$(OBJPREFIX)_libc.a $(OBJSO)
	
clean:
	rm -rf $(OBJSO) *.a
