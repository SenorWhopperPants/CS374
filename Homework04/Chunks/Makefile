CC      = mpicc
CFLAGS  = -DMPE_GRAPHICS -c -Wall
LFLAGS  = -o chunksMandel -L/usr/X11R6/lib -lmpe -lX11 -lm

chunksMandel: chunksMandel.o display.o
	$(CC) chunksMandel.o display.o $(LFLAGS)

chunksMandel.o: chunksMandel.c display.h
	$(CC) $(CFLAGS) chunksMandel.c 

display.o: display.c display.h
	$(CC) $(CFLAGS) display.c

run1:
	mpirun -np 1 ./chunksMandel

clean:
	rm -f chunksMandel a.out *.o *~ *# PI*

