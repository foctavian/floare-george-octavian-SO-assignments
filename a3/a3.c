#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <dirent.h>

#define PIPE_write "RESP_PIPE_55635"
#define PIPE_read "REQ_PIPE_55635"
#define ShM "/55Meom"

int main(int argc, char** argv) {

	int fd_write = -1;
	int fd_read = -1;
	unsigned int version = 55635;
	char variant[] = "VARIANT";
	char value[] = "VALUE";
	char success[] = "SUCCESS";
	char error[] = "ERROR";
	unsigned int size_sh = 0;
	volatile char* shared_mem = NULL;
	unsigned int offset, val;
	char file_name[];
	mkfifo(PIPE_write, 0600);

	fd_read = open(PIPE_read, O_RDONLY);
	if (fd_read == -1) {
		printf("ERROR\ncannot open the request pipe");
		return 1;
	}

	fd_write = open(PIPE_write, O_WRONLY);
	if (fd_write == -1) {
		perror("ERROR\ncannot create the response pipe");
		return 1;
	}


	//"BEGIN" test
	char begin[] = "BEGIN";
	unsigned int  size = strlen(begin);
	write(fd_write, &size, sizeof(char));
	for (int i = 0; i < strlen(begin); i++) {
		write(fd_write, &begin[i], sizeof(char));
	}
	printf("SUCCESS");

	for (;;) {
	size = 0;
	read(fd_read, &size, 1);
	char* request = (char*)calloc(size + 1, sizeof(char));
	if (request == NULL) return 1;

	for (int i = 0; i < size; i++) {
		read(fd_read, &request[i], sizeof(char));
	}
	request[size] = '\0';
	if (strncmp(request, "VARIANT", size) == 0) {
		unsigned int len = strlen(variant);
		write(fd_write, &len, 1);
		for (int i = 0; i < strlen(variant); i++) {
			write(fd_write, &variant[i], sizeof(char));
		}

		write(fd_write, &version, sizeof(version));

		len = strlen(value);
		write(fd_write, &len, 1);
		for (int i = 0; i < strlen(variant); i++) {
			write(fd_write, &value[i], sizeof(char));
		}
	}

	if (strncmp(request, "EXIT", size) == 0) {
		close(fd_write);
		close(fd_read);
		unlink(PIPE_write);
		return 0;
	}

	if (strncmp(request, "CREATE_SHM", size) == 0) {
		read(fd_read, &size_sh, sizeof(size_sh));

		int fd = shm_open(ShM, O_CREAT | O_RDWR, 0664);
		write(fd_write, &size, 1);
		for (int i = 0; i < size; i++) {
			write(fd_write, &request[i], sizeof(char));
		}

		if (fd < 0) {
			unsigned int len = strlen(error);
			write(fd_write, &len, 1);
			for (int i = 0; i < len; i++) {
				write(fd_write, &error[i], sizeof(char));
			}
		}
		else {
			ftruncate(fd, size_sh);
			shared_mem = (volatile char*)mmap(0, sizeof(char),
			                                  PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
			unsigned int len = strlen(success);
			write(fd_write, &len, 1);
			for (int i = 0; i < len; i++) {
				write(fd_write, &success[i], sizeof(char));
			}
		}
	}

	if (strncmp(request, "WRITE_TO_SHM", size) == 0) {
		read(fd_read, &offset, sizeof(offset));
		read(fd_read, &val, sizeof(val));

		write(fd_write, &size, 1);
		for (int i = 0; i < size; i++) {
			write(fd_write, &request[i], sizeof(char));
		}

		if (offset >= 0 && offset < size_sh) {
			shared_mem[offset-1] = val;
			unsigned int len = strlen(success);
			write(fd_write, &len, 1);
			for (int i = 0; i < len; i++) {
				write(fd_write, &success[i], sizeof(char));
			}
		}
		else {
			unsigned int len = strlen(error);
			write(fd_write, &len, 1);
			for (int i = 0; i < len; i++) {
				write(fd_write, &error[i], sizeof(char));
			}
		}
	}

	if(strncmp(request,"MAP_FILE", size) == 0){
		unsigned int file_size =0;
		read(fd_read,&file_size, 1 );

		file_name = malloc(file_size*sizeof(char));
		
	}


	free(request);
	}


	return 0;
}