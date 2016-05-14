
ROOT := ../..

all: vnl-io.a

include $(ROOT)/Makefile.rules

vnl-io.a: $(OBJ)
	$(AR) $(ARFLAGS) $@ $^
	install *.a $(ROOT)/lib/

clean:
	$(RM) bin/*
	$(RM) vnl-io.a
