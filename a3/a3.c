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
#define ShSIZE 1708215
#define ShM "/55Meom"

int fd_write = -1;
int fd_read = -1;


void sendString(char* string) {
	unsigned int len = strlen(string);
	write(fd_write, &len, sizeof(char));
	for (int i = 0; i < len; i++) {
		write(fd_write, &string[i], sizeof(char));
	}
}

int main(int argc, char** argv) {


	unsigned int version = 55635;
	char variant[] = "VARIANT";
	char write_to_shm[] = "WRITE_TO_SHM";
	char value[] = "VALUE";
	char success[] = "SUCCESS";
	char error[] = "ERROR";

	off_t file_size = 0;
	unsigned int size_sh = 0;
	void* shared_mem ;
	char* shared_file = NULL;
	unsigned int offset, val;
	char *file_name;

	mkfifo(PIPE_write, 0600);

	fd_read = open(PIPE_read, O_RDONLY);
	if (fd_read == -1) {
		printf("ERROR\ncannot open the request pipe");
		return 1;
	}

	fd_write = open(PIPE_write, O_WRONLY);
	if (fd_write == -1) {
		perror("ERROR\ncannot open the response pipe");
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
		//request[size] = '\0';
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
			munmap((void*)shared_mem, size_sh);
			munmap((void*)shared_file, file_size);
			shared_file = NULL;
			shared_mem = NULL;
			unlink(PIPE_write);
			shm_unlink(ShM);
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
				shared_mem = mmap(0, size_sh, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
				if (shared_mem == MAP_FAILED) {
					unsigned int len = strlen(error);
					write(fd_write, &len, 1);
					for (int i = 0; i < len; i++) {
						write(fd_write, &error[i], sizeof(char));
					}
				}
				else {
					unsigned int len = strlen(success);
					write(fd_write, &len, 1);
					for (int i = 0; i < len; i++) {
						write(fd_write, &success[i], sizeof(char));
					}
				}
			}
		}

		if (strncmp(request, "WRITE_TO_SHM", size) == 0) {
			read(fd_read, &offset, sizeof(offset));
			read(fd_read, &val, sizeof(val));
			write(fd_write, &size, 1);
			unsigned int len = 0;
			for (int i = 0; i < size; i++) {
				write(fd_write, &write_to_shm[i], sizeof(char));
			}

			if (offset < 0 || offset + sizeof(val) > ShSIZE) {
				len = strlen(error);
				write(fd_write, &len, 1);
				for (int i = 0; i < len; i++) {
					write(fd_write, &error[i], 1);
				}
			} else {

				*(unsigned int*)( shared_mem + offset) = val;
				len = strlen(success);
				write(fd_write, &len, 1);
				for (int i = 0; i < len; i++) {
					write(fd_write, &success[i], 1);

				}
			}
		}

		if (strncmp(request, "MAP_FILE", size) == 0) {
			unsigned int file_name_size = 0;
			read(fd_read, &file_name_size, 1);
			file_name = malloc((file_name_size) * sizeof(char));

			for(int i=0;i<file_name_size;i++){
				read(fd_read, &file_name[i], 1);
			}
			file_name[file_name_size] = '\0';
			sendString(request);
			if (strcmp(file_name, ".") != 0 &&
			        strcmp(file_name, "..") != 0 &&
			        strcmp(file_name, PIPE_write) != 0 &&
			        strcmp(file_name, PIPE_read) != 0) {

				int fd = open(file_name, O_RDONLY);

				if (fd == -1) {
					sendString(error);
				}
				else {
					file_size = lseek(fd, 0, SEEK_END);
					//lseek(fd, 0, SEEK_SET);
					ftruncate(fd, file_size);
					shared_file = ( char*)mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
					printf("a");

					if (shared_file == (void*) - 1) {
						sendString(error);

					} else {
						sendString(success);
					}
					close(fd);
				}
			} else {
				sendString(error);
			}
			free(file_name);
		}
		free(request);
	}




	return 0;
}