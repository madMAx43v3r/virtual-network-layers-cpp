
ROOT := ..
LDFLAGS := -lboost_coroutine -lboost_context -lboost_system -lpthread

all: test_stuff test_router test_atomics

include $(ROOT)/Makefile.rules

vnl:
	make -C ..

test_phy: bin/test_phy.o
	$(CXX) -o $@ $^ $(CFLAGS) $(LDFLAGS)

test_router: bin/test_router.o
	$(CXX) -o $@ $^ $(CFLAGS) $(LDFLAGS)

test_stuff: bin/test_stuff.o
	$(CXX) -o $@ $^ $(CFLAGS) $(LDFLAGS)

test_atomics: bin/test_atomics.o
	$(CXX) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf bin
	rm -f test_phy
	rm -f test_router
	rm -f test_stuff
	rm -f test_atomics

