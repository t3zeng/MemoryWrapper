# define some Makefile variables for the compiler and compiler flags
# to use Makefile variables later in the Makefile: $()
#
#  -g    adds debugging information to the executable file
#  -c    compiles without linking
#  -Wall turns on most, but not all, compiler warnings
#  -fpic compile as position independent code (since it's a lib)
CC = gcc
CFLAGS = -g -Wall
CFLAGS_LIB_COMPILE = -Wall -fPIC
LDFLAGS = -shared

default: debug # main

# Compile without wrapper
# main should use stdlib malloc, calloc, etc by default
# override with
main: main.c
	$(CC) $(CFLAGS) -o main main.c

# Compile with wrapper
debug: main.c tests.c wrapper
	$(CC) $(CFLAGS) -o main main.c tests.c ./libmemwrapper.so

# Create shared lib
#
wrapper: mem_wrapper.o
	$(CC) $(LDFLAGS) -o libmemwrapper.so mem_wrapper.o

# Compile memlib
#
mem_lib: mem_wrapper.c
	$(CC) $(CFLAGS_LIB_COMPILE) -o mem_wrapper.o mem_wrapper.c

# Cleans up files we generate
#
clean:
	$(RM) mem_wrapper *.o *~
	$(RM) libmemwrapper *.so *~
	$(RM) main *.exe *~
