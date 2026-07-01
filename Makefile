CC = cc
CFLAGS = -O3 -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE -D_DARWIN_C_SOURCE
SRCS = main.c qsc3.c range_coder.c context_model.c lz_engine.c
TARGET = qsc3_c
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) -lpthread -lm

zlib: $(SRCS)
	$(CC) $(CFLAGS) -DHAS_ZLIB -o $(TARGET) $(SRCS) -lpthread -lm -lz

debug: $(SRCS)
	$(CC) -O0 -g -Wall -Wextra -std=c11 -fsanitize=address -o $(TARGET)_dbg $(SRCS) -lpthread -lm

fast: $(SRCS)
	$(CC) -O3 -march=native -Wall -Wextra -std=c11 -o $(TARGET) $(SRCS) -lpthread -lm

install: $(TARGET)
	install -d "$(DESTDIR)$(BINDIR)"
	install -m 0755 $(TARGET) "$(DESTDIR)$(BINDIR)/qsc"

uninstall:
	rm -f "$(DESTDIR)$(BINDIR)/qsc"

clean:
	rm -f $(TARGET) $(TARGET)_dbg

.PHONY: all zlib debug fast install uninstall clean
