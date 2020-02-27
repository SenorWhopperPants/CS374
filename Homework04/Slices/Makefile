CC      = mpicc
CFLAGS  = -DMPE_GRAPHICS -c -Wall
LFLAGS  = -o slicesMandel -L/usr/X11R6/lib -lmpe -lX11 -lm

slicesMandel: slicesMandel.o display.o
	$(CC) slicesMandel.o display.o $(LFLAGS)

slicesMandel.o: slicesMandel.c display.h
	$(CC) $(CFLAGS) slicesMandel.c 

display.o: display.c display.h
	$(CC) $(CFLAGS) display.c

run1:
	mpirun -np 1 ./slicesMandel

clean:
	rm -f slicesMandel a.out *.o *~ *# PI*

