
ROOT := ..

all:
	make libvnl.a

include $(ROOT)/Makefile.rules

libvnl.a: $(OBJ)
	$(AR) $(ARFLAGS) $@ $^
	install libvnl.a $(ROOT)/lib/

clean:
	rm -rf bin
	rm -f libvnl.a
