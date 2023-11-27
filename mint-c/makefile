app := mint

CXX := clang
CXXFLAGS := -m32 -O3 -D __LINUX__

srcfiles := $(shell find . -name "*.cpp")
incfiles := $(shell find . -name "*.h")
LDLIBS   := -lm

all: $(app)

$(app): $(srcfiles) $(incfiles)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(app) $(srcfiles) $(LDLIBS)
	ls -l $(app)

clean:
	rm -f $(app)

bin: $(app)
	cp -u -p $(app) ~/.local/bin/
