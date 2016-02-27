
CFLAGS = -std=c++11 -g -O2 -I../include
LDFLAGS = -lboost_coroutine -lboost_context -lboost_system -lpthread

all: vnl clean test_stuff

vnl:
	make -C ..

test_stuff: bin/test_stuff.o
	$(CXX) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	$(RM) bin/*

bin/%.o: %.cpp
	$(CXX) -c -o $@ $< $(CFLAGS)
