
ROOT := ..
LDFLAGS := -lboost_coroutine -lboost_context -lboost_system -lpthread

all: vnl test_phy

include $(ROOT)/Makefile.rules

vnl:
	make -C ..

test_phy: bin/test_phy.o $(ROOT)/lib/*.a
	$(CXX) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	$(RM) bin/*
	$(RM) test_phy

