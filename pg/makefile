app := lsp

CXX := clang
CFLAGS := -m64 -O3 -D IS_LINUX

srcfiles := $(shell find . -name "*.c")
incfiles := $(shell find . -name "*.h")
LDLIBS   := -lm

all: $(app)

$(app): $(srcfiles) $(incfiles)
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $(app) $(srcfiles) $(LDLIBS)
	ls -l $(app)

clean:
	rm -f $(app)

test: $(app) test.jn
	./$(app) test.jn > test.asm
	fasm test.asm

run: $(app)
	./$(app)

bin: $(app)
	cp -u -p $(app) ~/bin/
