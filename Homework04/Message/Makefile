CC      = mpicc
CFLAGS  = -DMPE_GRAPHICS -c -Wall
LFLAGS  = -o messageMandel -L/usr/X11R6/lib -lmpe -lX11 -lm

messageMandel: messageMandel.o display.o
	$(CC) messageMandel.o display.o $(LFLAGS)

messageMandel.o: messageMandel.c display.h
	$(CC) $(CFLAGS) messageMandel.c 

display.o: display.c display.h
	$(CC) $(CFLAGS) display.c

run1:
	mpirun -np 1 ./messageMandel

clean:
	rm -f messageMandel a.out *.o *~ *# PI*

