CC = gcc
CFLAGS = -Wall -g
TARGET = paserRawDataFromSocket

# Default target
all: $(TARGET)

# Rule to compile the program
$(TARGET): paserRawDataFromSocket.c
	$(CC) $(CFLAGS) paserRawDataFromSocket.c -o $(TARGET)

# Clean up the build
clean:
	rm -f $(TARGET) *.o
