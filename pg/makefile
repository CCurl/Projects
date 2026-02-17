all: nlsp

lsp: lsp.c
	$(CC) -m64 -O3 -D IS_LINUX -o lsp lsp.c

nlsp: nlsp.c
	$(CC) -m64 -O3 -D IS_LINUX -o nlsp nlsp.c

l99: l99.c
	$(CC) -m64 -O3 -D IS_LINUX -o l99 l99.c

l3: l3.c
	$(CC) -m64 -O3 -D IS_LINUX -o l3 l3.c

clean:
	rm -f nlsp

run: nlsp
	./nlsp

bin: l99
	cp -u -p l99 ~/bin/
