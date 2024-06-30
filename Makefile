.phony: clean all

all:
	make -C meter
	make -C receiver
clean:
	make -C meter clean
	make -C receiver clean
