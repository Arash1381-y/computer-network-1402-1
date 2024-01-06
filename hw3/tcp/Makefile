cc := gcc


flags := -fopenmp -g -std=gnu11 -fdiagnostics-color=always -Wall -Wextra -Werror

files := $(wildcard *.c */*.c)
includes_header := ./CbNB ./connection_handler ./data_types ./http_parser ./socket
# add -I to the beginning of each include path
includes := $(foreach d, $(includes_header), -I$d)

dest := ./out/proxy_server

.PHONY: all compile

all: compile

compile: $(dest)

$(dest): $(files)
	$(cc) $(flags) $^ $(includes) -o $@

clean:
	rm -f proxy_server
