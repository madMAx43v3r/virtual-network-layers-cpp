
ROOT := ..

all:
	make vnl.a

include $(ROOT)/Makefile.rules

vnl.a: $(OBJ)
	$(AR) $(ARFLAGS) $@ $^
	install *.a $(ROOT)/lib/

clean:
	$(RM) bin/*
	$(RM) vnl.a
