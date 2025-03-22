#Use the make command to build the program. 
#By default, make will look for a Makefile in the 
#current directory and execute the all or default target.

CC = gcc

SRC = aesdsocket.c
TARGET = aesdsocket
OBJ = $(SRC:.c=.o)

#Cross-compilation support
TARGET_CC ?=
CC := $(TARGET_CC)$(CC)

#Build
all: $(TARGET)

#Compile the program
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)