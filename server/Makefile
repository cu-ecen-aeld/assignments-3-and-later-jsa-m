#Use the make command to build the program. 
#By default, make will look for a Makefile in the 
#current directory and execute the all or default target.

# CC = gcc

# SRC = aesdsocket.c
# TARGET = aesdsocket
# OBJ = $(SRC:.c=.o)

# #Cross-compilation support
# TARGET_CC ?=
# CC := $(TARGET_CC)$(CC)

# #Build
# all: $(TARGET)

# #Compile the program
# $(TARGET): $(OBJ)
# 	$(CC) $(OBJ) -o $(TARGET)

# # Compile source files into object files
# %.o: %.c
# 	$(CC) $(CFLAGS) -c $< -o $@

# clean:
# 	rm -f $(OBJ) $(TARGET)


#Updated for assignment 5 part 2 cross-compile arm for qemu
CC=$(TARGET_CC)

aesdsocket: aesdsocket.c
	$(CC) -c aesdsocket.c -o aesdsocket.o
	$(CC) aesdsocket.o -o aesdsocket

all: aesdsocket

clean:
	rm -f aesdsocket aesdsocket.o