CC=gcc
CFLAGS=-I/home/pis/src/licenta/ceph/build/include -L/home/pis/src/licenta/ceph/build/lib -lrados -g

all: client

client: client.c
	$(CC) $< $(CFLAGS) -o $@

clean:
	rm -rf client
