
objects = wdt_app.o 

hianDriverTest: $(objects) 

	arm-none-linux-gnueabi-gcc -o wdt_app $(objects)

wdt_app.o:wdt_app.c
	arm-none-linux-gnueabi-gcc -c wdt_app.c

clean:
	-rm wdt_app $(objects)
