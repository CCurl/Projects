CC := clang
CCFLAGS := -m32 -O3 -DIS_LINUX

all: s1

s1: s1.c
	$(CC) $(CCFLAGS) -o s1 s1.c
	ls -l s1

run: s1
	./s1

clean:
	rm -f s1

bin: s1
	cp -u -p s1 ~/bin/
