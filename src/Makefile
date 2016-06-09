
ROOT := ..

all:
	make libvnl.a

include $(ROOT)/Makefile.rules

libvnl.a: $(OBJ)
	$(AR) $(ARFLAGS) $@ $^
	install libvnl.a $(ROOT)/lib/

clean:
	$(RM) bin/*
	$(RM) libvnl.a
