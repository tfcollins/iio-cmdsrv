DESTDIR=/usr/local
CFLAGS += -Wall -Wextra -pedantic -std=gnu99 -s

EXEC = iio_cmdsrv
OBJS = iio_cmdsrv.o
#CC=microblazeel-unknown-linux-gnu-gcc

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS) -lm

install:
	install -d $(DESTDIR)/bin
	install ./$(EXEC) $(DESTDIR)/bin/
	install ./scripts/* $(DESTDIR)/bin/

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o
