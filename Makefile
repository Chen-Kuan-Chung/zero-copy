CC = gcc

all: file_copy

file_copy:
	$(CC) -o file_copy file_copy.c

clean:
	rm file_copy