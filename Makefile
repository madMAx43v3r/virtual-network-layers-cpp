
all:
	mkdir -p lib
	make -C src

clean:
	make -C src clean
	rm -rf lib
	
