# lightshow
This is an STM32F030F4-based light controller which uses five WS2812 RGB LEDs
to illuminate an object. Three potentiometers are used to control various
aspects of the lighting, including brightness, speed of variation and algorithm.
Additionally, an IR receiver allows remote control and a CdS cell can sense
ambient lighting conditions to adapt.

More details about this are available at the website here:

http://ebrombaugh.studionebula.com/embedded/lightshow/index.html

Building
--------

Just run
	make
	
Flashing
--------

1) Ensure openocd isn't running as a server already.

2) Make sure that your STM32F0 Discovery board is connected via USB and the
ST-LINK jumpers are set correctly (this example is for the on-board processor,
so 1-2 and 3-4 should be shorted)

3) run
	make flash

4) Done!

Debugging
---------

1) in a separate window start up openocd as a GDB server
	openocd -f openocd.cfg

2) run gdb with your favorite UI
	ddd --debugger arm-none-eabi-gdb main.elf

3) connect to the server within gdb
	target extended-remote localhost:3333

4) Start debugging!

License
-------

