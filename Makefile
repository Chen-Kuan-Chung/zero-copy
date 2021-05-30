CC = gcc

all: file_copy file_copy_client file_copy_server

file_copy:
	$(CC) -o file_copy file_copy.c

file_copy_client:
	$(CC) -o file_copy_client file_copy_client.c

file_copy_server:
	$(CC) -o file_copy_server file_copy_server.c

clean:
	rm file_copy file_copy_client file_copy_server