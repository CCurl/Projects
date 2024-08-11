app := wc

COMP := fasm
srcfiles := wc.asm         # $(shell find . -name "*.c")
# incfiles := opcodes.s      # $(shell find . -name "*.h")
opts := -d FOR_OS=LINUX

all: $(app)

$(app): $(srcfiles)
	$(COMP) $(srcfiles) $(opts)
	chmod +x $(app)
	ls -l $(app)

force: clean $(app)

clean:
	rm -f $(app)

bm:
	cat bm.txt | ./wc

test:
	cat test.txt | ./wc

bin: $(app)
	cp -u -p $(app64) ~/.local/bin/
