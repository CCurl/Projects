all: lsp

lsp: lsp.c
	$(CC) -m64 -O3 -D IS_LINUX -o lsp lsp.c

nlsp: nlsp.c
	$(CC) -m64 -O3 -D IS_LINUX -o nlsp nlsp.c

l99: l99.c
	$(CC) -m64 -O3 -D IS_LINUX -o l99 l99.c

clean:
	rm -f lsp

run: lsp
	./lsp

bin: lsp
	cp -u -p lsp ~/bin/
