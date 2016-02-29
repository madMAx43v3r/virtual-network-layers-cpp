
ROOT := ../..

all: vnl-util.a

include $(ROOT)/Makefile.rules

vnl-util.a: $(OBJ)
	$(AR) $(ARFLAGS) $@ $^
	install *.a $(ROOT)/lib/

clean:
	$(RM) bin/*
	$(RM) vnl-util.a
