CC=gcc
ODIR=obj

all: master child

master: master.c
	$(CC) -g -o oss master.c -pthread
	
child: child.c
	$(CC) -g -o user child.c -pthread

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~