CFLAGS := -ggdb3 -O2 -Wall -Wextra -std=c11
CFLAGS += -Wvla
CPPFLAGS := -D_DEFAULT_SOURCE

PROGS := primes xargs ps timeout

all: $(PROGS)

xargs: xargs.o
primes: primes.o
ps: ps.o
timeout: timeout.o

test:
	./tests/run $(realpath .)

format: .clang-files .clang-format
	xargs -r clang-format -i <$<

clean:
	rm -f $(PROGS) *.o core vgcore.*

.PHONY: all clean format test
