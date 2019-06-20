TARGET = SCFS
LIBS = -lfuse3 -lpthread
CC = gcc
CFLAGS = -g -Wall
IDIR = include
ODIR = out

default: test_fuse
all: test_image test_bitmap test_superblock test_inode test_scfs test_fuse

test_image: src/debugprintf.c src/image.c test/test_image.c
	$(CC) $(CFLAGS) -I$(IDIR) $^ -o $(ODIR)/$@

test_bitmap: src/debugprintf.c src/image.c src/bitmap.c test/test_bitmap.c
	$(CC) $(CFLAGS) -I$(IDIR) $^ -o $(ODIR)/$@

test_superblock: src/debugprintf.c src/image.c src/superblock.c test/test_superblock.c
	$(CC) $(CFLAGS) -I$(IDIR) $^ -o $(ODIR)/$@

test_inode: src/debugprintf.c src/image.c src/bitmap.c src/superblock.c src/block.c src/inode.c test/test_inode.c
	$(CC) $(CFLAGS) -I$(IDIR) $^ -o $(ODIR)/$@

test_scfs: src/debugprintf.c src/image.c src/bitmap.c src/superblock.c src/block.c src/inode.c src/scfs.c test/test_scfs.c
	$(CC) $(CFLAGS) $(LIBS) -I$(IDIR) -I/usr/include/fuse3 $^ -o $(ODIR)/$@

test_fuse: src/debugprintf.c src/image.c src/bitmap.c src/superblock.c src/block.c src/inode.c src/scfs.c test/test_fuse.c
	$(CC) $(CFLAGS) $(LIBS) -I$(IDIR) -I/usr/include/fuse3 $^ -o $(ODIR)/$@

clean:
	-rm $(ODIR)/*
