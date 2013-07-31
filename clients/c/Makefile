CFLAGS += -Wall -Wextra -pedantic -std=gnu99 -O2

EXEC = lib_iio_cmdsrv
OBJS = lib_iio_cmdsrv.o main.o
#CC=microblazeel-unknown-linux-gnu-gcc

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS) -lanl

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o
