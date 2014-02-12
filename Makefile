CC=gcc -g
CFLAGS=$(shell pkg-config --cflags gstreamer-1.0 glib-2.0 gstreamer-pbutils-1.0)
LIBS=$(shell pkg-config --libs gstreamer-1.0 glib-2.0 gstreamer-pbutils-1.0)
SOURCES=src/keyboard_thread.c src/player_priv.c src/player.c player_testsuite.c 
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=player

CFLAGS+=-I./include

all: $(SOURCES) $(EXECUTABLE)

install:
	make all; \
		cp ./player /usr/local/bin/player

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LIBS); \
		make clean

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@  

clean:
	rm -rf ./src/*.o ./*.o
