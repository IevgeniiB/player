CC=gcc
CFLAGS=-c -I/usr/include/gstreamer-1.0 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I./include
LIBS=-lgstreamer-1.0 -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lglib-2.0
SOURCES=player_testsuite.c src/player.c src/player_priv.c src/keyboard_thread.c 
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=player

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf ./src/*.o ./*.o
