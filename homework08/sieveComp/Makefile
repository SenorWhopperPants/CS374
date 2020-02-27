TARGET = sieve
CC     = mpicc
CFLAGS = -Wall -ansi -pedantic -std=c99
LFLAGS = -o $(TARGET) -lm

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) $(TARGET).c $(LFLAGS)

clean:
	rm -f $(TARGET)  *~ *# *.o

