# Compiler
CC=gcc
# Flags for compilation
CFLAGS=-Wall
# Executable file name
EXEC=sgf
# Source files folder
SRCDIR=src
# Build folder
BUILDDIR=build
# Source files names
SRC=$(shell find $(SRCDIR) -type f -name "*.c")
# .o files names
OBJ=$(SRC:.c=.o)

# Executable
all: $(EXEC)

# Create executable file with all objects files + main.c
$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) main.c $^ -o $@ -g 

# Create each object file with its source file
%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -Iheaders -c $< -o $@ -g

# Delete objects file
clean:
	rm -r *.o
