CC = gcc
CFLAGS = -Wall -pedantic
objects = supermercato.o cassa.o cliente.o direttore.o gestore.o parsing.o queue.o

.PHONY: all clean test1 test2

exesm.out: $(objects)
	$(CC) -g $(CFLAGS) $(objects) -o exesm.out -lpthread

supermercato.o: supermercato.c util.h
cassa.o: cassa.c util.h
cliente.o: cliente.c util.h
direttore.o: direttore.c util.h
gestore.o: gestore.c util.h
parsing.o: parsing.c util.h
queue.o: queue.c util.h

all: exesm.out

test1:
	make clean
	make all
	bash test1.sh

test2:
	make clean
	make all
	bash test2.sh
	
clean:
	rm -f *.txt *.o *.out
