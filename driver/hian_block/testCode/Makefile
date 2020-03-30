
objects = hian_debug.o 

hian_debug: $(objects) 

	arm-none-linux-gnueabi-gcc -o hian_debug $(objects)

hian_debug.o:hian_debug.c
	arm-none-linux-gnueabi-gcc -c hian_debug.c

clean:
	-rm hian_debug $(objects)
