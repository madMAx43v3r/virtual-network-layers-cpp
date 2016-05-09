
ROOT := ..
LDFLAGS := -lboost_coroutine -lboost_context -lboost_system -lpthread

all: test_stuff test_phy_router

include $(ROOT)/Makefile.rules

vnl:
	make -C ..

test_phy: bin/test_phy.o
	$(CXX) -o $@ $^ $(CFLAGS) $(LDFLAGS)

test_phy_router: bin/test_phy_router.o
	$(CXX) -o $@ $^ $(CFLAGS) $(LDFLAGS)

test_stuff: bin/test_stuff.o
	$(CXX) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	$(RM) bin/*
	$(RM) test_phy
	$(RM) test_phy_router
	$(RM) test_stuff

