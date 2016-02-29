
all:
	make -C src

clean:
	make -C src clean
	$(RM) lib/*
	
