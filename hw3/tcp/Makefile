cc := gcc


flags := -fopenmp -g -std=gnu11 -fdiagnostics-color=always

files := $(wildcard *.c */*.c)
includes_header := ./CbNB ./connection_handler ./data_types ./http_parser ./socket

# add -I to the beginning of each include path
includes := $(foreach d, $(includes_header), -I$d)


.PHONY: all


all: $(files)
	$(cc) $(flags) $(files) $(includes) -o proxy_server

clean:
	rm -f proxy_server