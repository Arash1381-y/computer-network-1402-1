CC = gcc
CFLAGS = -g -Wall -std=c99 -fopenmp


files = $(wildcard *.c)

.PHONY: clean seq all parallel

all: seq parallel

# if out dir not exist, create it
$(shell mkdir -p out)


seq: $(files)
	$(CC) $(CFLAGS) -o ./out/$@ $^ -DPORT=1010

parallel: $(files)
	$(CC) $(CFLAGS) -o ./out/$@ $^ -DPARALLEL_MODE -DPORT=3030

clean:
	rm -f ./out/seq ./out/parallel

