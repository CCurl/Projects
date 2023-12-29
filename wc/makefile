app64 := wc

COMP := fasm
srcfiles := wc.s           # $(shell find . -name "*.c")
incfiles := opcodes.s      # $(shell find . -name "*.h")
LDLIBS   := -lm

all: $(app64)

$(app64): $(srcfiles)
	$(COMP) $(srcfiles)
	chmod +x $(app64)
	ls -l $(app64)

force: clean $(app64)

clean:
	rm -f $(app64)

bm:
	cat bm.txt | ./wc

test:
	cat test.txt | ./wc

bin: $(app64)
	cp -u -p $(app64) ~/.local/bin/
