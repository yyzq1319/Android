
objects = main.o leds.o

hianDriverTest: $(objects) 

	arm-none-linux-gnueabi-gcc -o hianDriverTest $(objects)

main.o:main.c
	arm-none-linux-gnueabi-gcc -c main.c

leds.o:leds.c
	arm-none-linux-gnueabi-gcc -c leds.c leds.h

clean:
	-rm hianDriverTest $(objects)
