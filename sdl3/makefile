app := sdl3-example

CC := clang
SDL3_CFLAGS := $(shell pkg-config --cflags sdl3 2>/dev/null)
SDL3_LIBS := $(shell pkg-config --libs sdl3 2>/dev/null)
CFLAGS := -m64 -O2 -Wall -Wextra -D IS_LINUX $(SDL3_CFLAGS)
LDLIBS := $(SDL3_LIBS)

all: $(app)

$(app): main.c
	@if [ -z "$(SDL3_CFLAGS)" ] || [ -z "$(SDL3_LIBS)" ]; then \
		echo "SDL3 not found. Install SDL3 development files so pkg-config can find 'sdl3'."; \
		exit 1; \
	fi
	$(CC) $(CFLAGS) -o $(app) main.c $(LDLIBS)
	ls -l $(app)

run: $(app)
	./$(app)

clean:
	rm -f $(app)
