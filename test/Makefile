
ROOT := ..
LDFLAGS := -lboost_coroutine -lboost_context -lboost_system -lpthread

all: test_stuff

include $(ROOT)/Makefile.rules

vnl:
	make -C ..

test_phy: bin/test_phy.o $(ROOT)/lib/*.a
	$(CXX) -o $@ $^ $(CFLAGS) $(LDFLAGS)

test_stuff: bin/test_stuff.o
	$(CXX) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	$(RM) bin/*
	$(RM) test_phy
	$(RM) test_stuff

