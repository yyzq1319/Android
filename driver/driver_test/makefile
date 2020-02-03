
objects = main.o leds.o read_gpio.o

hianDriverTest: $(objects) 

	arm-none-linux-gnueabi-gcc -o hianDriverTest $(objects)

main.o:main.c
	arm-none-linux-gnueabi-gcc -c main.c

leds.o:leds.c
	arm-none-linux-gnueabi-gcc -c leds.c leds.h
read_gpio.o:read_gpio.c
	arm-none-linux-gnueabi-gcc -c read_gpio.c read_gpio.h

clean:
	-rm hianDriverTest $(objects)
