CC = gcc
CFLAGS = -Wall -g
TARGET = paserRawDataFromSocket

# Default target
all: $(TARGET)

# Rule to compile the program
$(TARGET): paserRawDataFromSocket.c
	$(CC) $(CFLAGS) paserRawDataFromSocket.c -o $(TARGET)
ipc:
	gcc mq_notify_send_receive.c -o mq_notify_example -lrt -pthread

# Clean up the build
clean:
	rm -f $(TARGET) *.o
