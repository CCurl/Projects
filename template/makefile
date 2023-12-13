CXX := clang
CXXFLAGS := -m32 -O3 -D IS_LINUX

LDLIBS   :=

all: t1 t2

t1: t1.c
	$(CXX) $(CXXFLAGS) t1.c -o t1
	ls -l t1

t2: t2.c
	$(CXX) $(CXXFLAGS) t2.c -o t2
	ls -l t2

clean:
	rm -f t1 t2

force: clean t1 t2

test1: $(app)
	t1 test1.txt

test2: $(app)
	t2 test2.txt
