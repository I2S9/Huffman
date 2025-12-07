CC = gcc
LIBS = -lm

COMPRESS = compress
DECOMPRESS = decompress

COMPRESS_SRC = src/compress.c
DECOMPRESS_SRC = src/decompress.c

all: $(COMPRESS) $(DECOMPRESS)

compress: $(COMPRESS_SRC)
	$(CC) $(COMPRESS_SRC) -o $(COMPRESS) $(LIBS)

decompress: $(DECOMPRESS_SRC)
	$(CC) $(DECOMPRESS_SRC) -o $(DECOMPRESS) $(LIBS)

clean:
	rm -f $(COMPRESS) $(DECOMPRESS)

.PHONY: all clean