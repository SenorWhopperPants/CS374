TARGET = sieve2
CC     = mpicc
CFLAGS = -Wall -ansi -pedantic -std=c99 -fopenmp -lgomp
LFLAGS = -o $(TARGET) -lm

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) $(TARGET).c $(LFLAGS)

clean:
	rm -f $(TARGET)  *~ *# *.o

