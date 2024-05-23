.phony: clean all

all:
	make -C 01_led_blink

clean:
	make -C 01_led_blink clean
