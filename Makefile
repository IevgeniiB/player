CC=gcc -g
CFLAGS=-c -I/usr/include/gstreamer-1.0 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I./include
LIBS=-lgstreamer-1.0 -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lglib-2.0 -lgstpbutils-1.0
SOURCES=player_testsuite.c src/player.c src/player_priv.c src/keyboard_thread.c 
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=player

all: $(SOURCES) $(EXECUTABLE)

install:
	make all; \
		cp ./player /usr/local/bin/player

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LIBS); \
		make clean

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf ./src/*.o ./*.o
