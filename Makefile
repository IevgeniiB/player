CC=gcc
CFLAGS=-pthread -I/usr/include/gstreamer-1.0 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
LIBS=-pthread -lgstreamer-1.0 -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lglib-2.0
SOURCES=player.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=player

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES) 
	$(CC) -o $@ $< $(CFLAGS) $(LIBS)
