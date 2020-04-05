
objects = adc_app.o 

hianDriverTest: $(objects) 

	arm-none-linux-gnueabi-gcc -o adc_app $(objects)

adc_app.o:adc_app.c
	arm-none-linux-gnueabi-gcc -c adc_app.c

clean:
	-rm adc_app $(objects)
