app64 := wc

CXX := clang
CXXFLAGS32 := -m32 -O2 
CXXFLAGS64 := -m64 -O3 

srcfiles := $(shell find . -name "*.c")
incfiles := $(shell find . -name "*.h")
LDLIBS   := -lm

all: $(app64)

$(app64): wc.s opcodes.s
	fasm wc.s
	chmod +x wc
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
