# This is a host tool, won't use the makefile infra of DiegOS
.PHONY: all clean

OBJSC = ../fat_format.o ../fat_internals.o ../fat_names.o
OBJSB = floppy_access.o main_floppy.o mkimgfd.o
OBJSA = disk_access.o main.o mkimg.o ../mbr.o
CFLAGS += -O1 -Wall -Wextra -Winline -Wcast-align -g3

all: $(OBJSC) $(OBJSB) $(OBJSA)
	$(CC) -o mkimage $(OBJSA) $(OBJSC)
	$(CC) -o mkfloppy $(OBJSB) $(OBJSC)

clean:
	rm -f *.o mkimage mkfloppy
