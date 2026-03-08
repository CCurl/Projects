CC := clang
CCFLAGS := -m32 -O3 -DIS_LINUX

all: s1

s1: s1.c
	$(CC) $(CCFLAGS) -o s1 s1.c
	ls -l s1

r1: s1
	./s1

run: $(app)
	./$(app)

clean:
	rm -f $(app)

bin: $(app)
	cp -u -p $(app) ~/bin/
