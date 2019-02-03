CC = g++
AR = ar
CFLAGS = -Wall -Iinclude -O2

ifdef DEBUG
	CFLAGS += -ggdb
endif

LDFLAGS = -lsbn -Llib
LIB = lib/libsbn.a
TESTPROG = bin/sbntest
TESTSRC = test/sbntest.cpp

SRC = $(wildcard src/*.cpp)
INCLUDES = $(wildcard include/*.h)
OBJ = $(patsubst %.cpp, %.o, $(SRC))
TESTSRC = $(wildcard test/*.cpp)
TESTOBJ = $(patsubst %.cpp, %.o, $(TESTSRC))

all: $(LIB)

$(LIB): $(OBJ)
	$(AR) cq $@ $(OBJ)
	ranlib $@

$(OBJ): src/%.o: src/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(TESTOBJ): test/%.o: test/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

doc: doc/html/index.html

doc/html/index.html: $(SRC) $(INCLUDES)
	doxygen doc/config

check: $(LIB) $(TESTPROG)
	$(TESTPROG)

$(TESTPROG): $(TESTOBJ) $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(TESTOBJ) -o $(TESTPROG)

clean:
	rm -f src/*.o test/*.o $(LIB) $(TESTPROG) bin/sbntest.exe
	rm -rf doc/html doc/latex
