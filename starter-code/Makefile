CC=gcc
# CFLAGS=-ggdb3 -c -Wall -Werror -std=gnu99
# LDFLAGS=-pthread
CFLAGS=-ggdb3 -c -Wall -Werror -std=gnu99 -g -fsanitize=address
LDFLAGS=-pthread -fsanitize=address
# SOURCES=proxyserver.c safequeue.c
SOURCES=safequeue.c proxyserver.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=proxyserver

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)
