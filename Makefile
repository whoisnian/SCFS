TARGET = shel
LIBS = -lfuse3 -lpthread
CC = gcc
CFLAGS = -g -Wall
IDIR = include
ODIR = out

default: $(TARGET) ls
all: default

#ls: src/ls.c
#	$(CC) $(CFLAGS) $(LIBS) -I$(IDIR) $< -o $(ODIR)/bin/$@

$(TARGET): src/main.c
	mkdir -p $(ODIR)/bin
	$(CC) $(CFLAGS) $(LIBS) -I$(IDIR) $< -o $(ODIR)/$@

clean:
	-rm -rf $(ODIR)
