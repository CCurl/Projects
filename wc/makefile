app := wc32

COMP := fasm
srcfiles := wc32.asm
opts := -d FOR_OS=LINUX

all: $(app)

$(app): $(srcfiles)
	$(COMP) $(opts) $(srcfiles)
	chmod +x $(app)
	ls -l $(app)

force: clean $(app)

clean:
	rm -f $(app)

bm:
	cat bm.txt | $(app)

test:
	cat test.txt | $(app)

bin: $(app)
	cp -u -p $(app) ~/.local/bin/
