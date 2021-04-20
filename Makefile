CC=gcc
CFLAGS=-fsanitize=address -c -Wall -I. -fpic -g -fbounds-check
LDFLAGS=-L. -fsanitize=address
LIBS=-lcrypto

OBJS=tester.o util.o mdadm.o cache.o

%.o:	%.c %.h
	$(CC) $(CFLAGS) $< -o $@

tester:	$(OBJS) jbod.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(OBJS) tester
