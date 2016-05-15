
ROOT := ..

all:
	make -C phy
	make -C util
	make vnl.a

include $(ROOT)/Makefile.rules

vnl.a: $(OBJ)
	$(AR) $(ARFLAGS) $@ $^
	install *.a $(ROOT)/lib/

clean:
	make -C phy clean
	make -C util clean
	$(RM) bin/*
	$(RM) vnl.a
