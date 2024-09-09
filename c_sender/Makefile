# Variables
CC = gcc
CFLAGS = -I.. -Wall -Wextra -std=c11
SRC = test_sender.c ../telemetry_udp.c
OBJ = $(SRC:.c=.o)
TARGET = test_sender

# Default target
all: $(TARGET)

# Compile target
$(TARGET): $(OBJ)
	$(CC) -o $(TARGET) $(OBJ)

# Rule for compiling .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJ) $(TARGET)

# Phony targets to avoid conflicts with files of the same name
.PHONY: all clean
