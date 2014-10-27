objects = rte_ring_main.o rte_ring.o
CC = gcc

all: $(objects)
	$(CC) -g -o main $(objects) -lpthread

$(objects): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f main *.o
