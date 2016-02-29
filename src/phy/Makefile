
ROOT := ../..

all: vnl-phy.a

include $(ROOT)/Makefile.rules

vnl-phy.a: $(OBJ)
	$(AR) $(ARFLAGS) $@ $^
	install *.a $(ROOT)/lib/

clean:
	$(RM) bin/*
	$(RM) vnl-phy.a
