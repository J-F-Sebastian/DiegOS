# This is a host tool, won't use the makefile infra of DiegOS
.PHONY: all clean

OBJS = disk_access.o main.o mkimg.o mkimgfd.o  ../fat_format.o ../fat_internals.o ../mbr.o ../fat_names.o
CFLAGS += -O1 -Wall -Wextra -Winline -Wcast-align -g

all: $(OBJS)
	$(CC) -o mkimage $(OBJS)

clean:
	rm -f *.o mkimage
