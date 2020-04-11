
objects = moduleTest.o 

moduleTest: $(objects) 

	arm-none-linux-gnueabi-gcc -o moduleTest $(objects)

moduleTest.o:moduleTest.c
	arm-none-linux-gnueabi-gcc -c moduleTest.c

clean:
	-rm moduleTest $(objects)
