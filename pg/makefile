app := cw
app32 := cw-32

CXX := clang
CXXFLAGS := -m64 -O3 -D IS_LINUX
C32FLAGS := -m32 -O3 -D IS_LINUX

srcfiles := $(shell find . -name "*.c")
incfiles := $(shell find . -name "*.h")
LDLIBS   := -lm

all: $(app) $(app32)

$(app): $(srcfiles) $(incfiles)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(app) $(srcfiles) $(LDLIBS)
	ls -l $(app)

$(app32): $(srcfiles) $(incfiles)
	$(CXX) $(C32FLAGS) $(LDFLAGS) -o $(app32) $(srcfiles) $(LDLIBS)
	ls -l $(app32)

clean:
	rm -f $(app) $(app32)

test: $(app)
	./$(app) test.wc

bin: $(app)
	cp -u -p $(app) ~/bin/
