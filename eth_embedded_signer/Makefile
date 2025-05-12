CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude
LDFLAGS =
SOURCES = src/main.c src/crypto.c src/rlp.c src/transaction.c
TARGET = eth_signer

all: $(TARGET)

$(TARGET): $(SOURCES)
	@mkdir -p build
	$(CC) $(CFLAGS) -o build/$(TARGET) $(SOURCES) $(LDFLAGS)
	@echo "Build successful! Executable created at: build/$(TARGET)"

clean:
	rm -rf build

run: all
	./build/$(TARGET)

.PHONY: all clean run 