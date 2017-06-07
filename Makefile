# NAME: Nikhil Bhatia, Chaoran Lin
# UID: 104132751, 004674598
# EMAIL: nbhatia823@ucla.edu, linmc@ucla.edu

CC=gcc
CFLAGS=-lm -o
EXECNAME=lab3b

default: lab3b.c
	$(CC) lab3b.c $(CFLAGS) $(EXECNAME)
	chmod +x ./$(EXECNAME)

dist:
	tar -cvzf lab3b-104132751.tar.gz lab3b.c Makefile README

clean:
	rm -f $(EXECNAME) *.tar.gz *~
