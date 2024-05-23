.phony: clean all

all:
	make -C 01_led_blink
	make -C meter

clean:
	make -C 01_led_blink clean
	make -C meter clean
