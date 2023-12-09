app := pg

CXX := clang
CXXFLAGS := -m32 -O3 -D IS_LINUX

srcfiles := $(shell find . -name "*.c")
incfiles := $(shell find . -name "*.h")
LDLIBS   :=

all: $(app)

$(app): $(srcfiles) $(incfiles)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(app) $(srcfiles) $(LDLIBS)
	ls -l $(app)

clean:
	rm -f $(app)

force: clean $(app)

test1: $(app)
	./$(app) test1.txt

test2: $(app)
	cat test2.txt | ./$(app)

bin: $(app)
	cp -u -p $(app) ~/.local/bin/
