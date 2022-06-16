TARGET = fib
TOOLCHAIN = riscv64-unknown-elf-
CC = $(TOOLCHAIN)gcc
LDSCRIPT = ../../scripts/link.ld
LDFLAGS = -T $(LDSCRIPT) -nostdlib
CFLAGS = -march=rv64i -mabi=lp64
CRT = ../../scripts/crt0.S
SRC = fib.c

OBJS = fib.o crt0.o

$(TARGET): $(OBJS) $(LDSCRIPT)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

fib.o: fib.c
crt0.o: $(CRT)
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	-$(RM) $(OBJS) $(TARGET)

.PHONY: clean
