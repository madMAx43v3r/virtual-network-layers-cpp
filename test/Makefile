
CFLAGS = -std=c++11 -g -O2 -I../include
LDFLAGS = -lboost_coroutine -lboost_context -lboost_system -lpthread

all: vnl clean test_tcp test_stuff test_link test_local

vnl:
	make -C ..

test_stuff: bin/test_stuff.o
	$(CXX) -o $@ $^ $(CFLAGS) $(LDFLAGS)

test_link: bin/test_link.o
	$(CXX) -o $@ $^ $(CFLAGS) $(LDFLAGS)

test_local: bin/test_local.o
	$(CXX) -o $@ $^ $(CFLAGS) $(LDFLAGS)

test_tcp: bin/test_tcp.o
	$(CXX) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	$(RM) bin/*

bin/%.o: %.cpp
	$(CXX) -c -o $@ $< $(CFLAGS)
