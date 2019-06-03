TARGET = SCFS
LIBS = -lfuse3 -lpthread
CC = gcc
CFLAGS = -g -Wall
IDIR = include
ODIR = out

default: shel ls
all: default

#ls: src/ls.c
#	$(CC) $(CFLAGS) $(LIBS) -I$(IDIR) $< -o $(ODIR)/bin/$@

shel: src/shel.c
	mkdir -p $(ODIR)/bin
	$(CC) $(CFLAGS) $(LIBS) -I$(IDIR) $< -o $(ODIR)/$@

clean:
	-rm -rf $(ODIR)
