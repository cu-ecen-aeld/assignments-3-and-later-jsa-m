CC = $(CROSS_COMPILE)gcc
TARGET =writer #Target binary name
SRC = writer.c #Source file
OBJ = $(SRC:.c=.o) #Set object file
CFLAGS = -Wall -Werror -g #Enables all warnings, treat warnings as errors and add debug information)
#Rule to build the target binary
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)
#Rule to compile .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
#Clean target to remove .o files and the binary
clean:
	rm -f $(TARGET) $(OBJ) 
#Cross-compilation support
cross:
	$(MAKE) CROSS_COMPILE=aarch64-none-linux-gnu-
.PHONY: all clean cross #tells make that all, clean and cross are not filenames and should be excuted
