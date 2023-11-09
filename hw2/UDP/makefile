CC = gcc
CFLAGS = -g -Wall -std=c99 -fopenmp


files = $(wildcard *.c)

.PHONY: clean seq all parallel

all: seq parallel

# if out dir not exist, create it
$(shell mkdir -p out)


seq: $(files)
	$(CC) $(CFLAGS) -o ./out/$@ $^

parallel: $(files)
	$(CC) $(CFLAGS) -o ./out/$@ $^ -DPARALLEL_MODE

clean:
	rm -f ./out/seq ./out/parallel

