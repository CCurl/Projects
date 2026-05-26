app := sdl3-example

CC := clang
CFLAGS := -m64 -O2 -Wall -Wextra -D IS_LINUX $(shell pkg-config --cflags sdl3)
LDLIBS := $(shell pkg-config --libs sdl3)

all: $(app)

$(app): main.c
$(CC) $(CFLAGS) -o $(app) main.c $(LDLIBS)
ls -l $(app)

run: $(app)
./$(app)

clean:
rm -f $(app)
