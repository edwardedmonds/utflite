CC = clang
CFLAGS = -Wall -Wextra -pedantic -std=c17 -O3
CFLAGS_DEBUG = -Wall -Wextra -pedantic -std=c17 -g -O0
AR = ar
ARFLAGS = rcs

PREFIX = /usr/local
INCLUDEDIR = $(PREFIX)/include
LIBDIR = $(PREFIX)/lib

SRC = src/utflite.c
OBJ = src/utflite.o
LIB = libutflite.a
HEADER = src/utflite.h

.PHONY: all clean install uninstall test test-single debug

all: $(LIB)

$(LIB): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

src/utflite.o: src/utflite.c $(HEADER)
	$(CC) $(CFLAGS) -I src -c $< -o $@

clean:
	rm -f $(OBJ) $(LIB)
	rm -f test/test_utflite test/test_single

install: $(LIB) $(HEADER)
	install -d $(INCLUDEDIR)
	install -d $(LIBDIR)
	install -m 644 $(HEADER) $(INCLUDEDIR)/utflite.h
	install -m 644 utflite.h $(INCLUDEDIR)/utflite_single.h
	install -m 644 $(LIB) $(LIBDIR)/

uninstall:
	rm -f $(INCLUDEDIR)/utflite.h
	rm -f $(INCLUDEDIR)/utflite_single.h
	rm -f $(LIBDIR)/libutflite.a

# Test using static library
test: $(LIB) test/test_utflite.c
	$(CC) $(CFLAGS_DEBUG) -I src test/test_utflite.c -L. -lutflite -o test/test_utflite
	./test/test_utflite

# Test using single-header version
test-single: test/test_utflite.c utflite.h
	$(CC) $(CFLAGS_DEBUG) -DUTFLITE_SINGLE_HEADER test/test_utflite.c -o test/test_single
	./test/test_single

# Debug build
debug: CFLAGS = $(CFLAGS_DEBUG)
debug: clean $(LIB)
