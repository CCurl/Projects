CXX := clang
CXXFLAGS :=  -m32 -D_LINUX

objects := $(shell find . -name "*.o")

all: f-comp f-dis f-vm

rebuild: clean f-comp f-dis f-vm

f-comp: forth-compiler.o forth-vm.o string.o
	$(CXX) $(CXXFLAGS) -o f-comp forth-compiler.o forth-vm.o string.o

f-dis: forth-dis.o forth-vm.o string.o
	$(CXX) $(CXXFLAGS) -o f-dis forth-dis.o forth-vm.o string.o

f-vm: forth.o forth-vm.o string.o
	$(CXX) $(CXXFLAGS) -o f-vm forth.o forth-vm.o string.o

forth-compiler.o: forth-compiler.c forth-vm.o
	$(CXX) $(CXXFLAGS) -c forth-compiler.c
 
forth-dis.o: forth-dis.c string.c string.h
	$(CXX) $(CXXFLAGS) -c forth-dis.c
 
forth-vm.o: forth-vm.c string.o
	$(CXX) $(CXXFLAGS) -c forth-vm.c
 
forth.o: forth.c forth-vm.o
	$(CXX) $(CXXFLAGS) -c forth.c

string.o: string.c string.h
	$(CXX) $(CXXFLAGS) -c string.c

clean:
	rm -f $(objects)
