.phony: clean all

all:
	make -C 01_led_blink
	make -C 06_shaft_encoder_exercise
	make -C meter

clean:
	make -C 01_led_blink clean
	make -C meter clean
	make -C 06_shaft_encoder_exercise
