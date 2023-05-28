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
#define MEM_ALLIGNMENT 4096
#define ShM "/55Meom"

int fd_write = -1;
int fd_read = -1;
int _version = 0;
char _magic = 0;
char _sect_nr = 0;
int _sect_offset = 0;
int _sect_size = 0;
short _header_size = 0;

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
	char value[] = "VALUE";
	char success[] = "SUCCESS";
	char error[] = "ERROR";

	off_t file_size = 0;
	unsigned int size_sh = 0;
	void* shared_mem ;
	void* shared_file = NULL;
	unsigned int offset, val;
	char *file_name;
	unsigned int file_offset, no_of_bytes, section_no, logical_offset;
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
			sendString(variant);

			write(fd_write, &version, sizeof(version));


			sendString(value);
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
			free(file_name);
			return 0;
		}

		if (strncmp(request, "CREATE_SHM", size) == 0) {
			read(fd_read, &size_sh, sizeof(size_sh));

			int fd = shm_open(ShM, O_CREAT | O_RDWR, 0664);

			sendString("CREATE_SHM");

			if (fd < 0) {
				sendString(error);
			}
			else {
				ftruncate(fd, size_sh);
				shared_mem = mmap(0, size_sh, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
				if (shared_mem == MAP_FAILED) {
					sendString(error);

				}
				else {
					sendString(success);

				}
			}
		}

		if (strncmp(request, "WRITE_TO_SHM", size) == 0) {
			read(fd_read, &offset, sizeof(offset));
			read(fd_read, &val, sizeof(val));
			sendString("WRITE_TO_SHM");

			if (offset < 0 || offset + sizeof(val) > ShSIZE) {
				sendString(error);
			} else {

				*(unsigned int*)( shared_mem + offset) = val;
				sendString(success);
			}
		}

		if (strncmp(request, "MAP_FILE", size) == 0) {
			unsigned int file_name_size = 0;
			read(fd_read, &file_name_size, 1);
			file_name = malloc((file_name_size) * sizeof(char));
			int fd = -1;
			for (int i = 0; i < file_name_size; i++) {
				read(fd_read, &file_name[i], 1);
			}
			file_name[file_name_size] = '\0';
			sendString(request);
			if (strcmp(file_name, ".") != 0 &&
			        strcmp(file_name, "..") != 0 &&
			        strcmp(file_name, PIPE_write) != 0 &&
			        strcmp(file_name, PIPE_read) != 0) {

				fd = open(file_name, O_RDONLY);

				if (fd == -1) {
					sendString(error);
				}
				else {
					file_size = lseek(fd, 0, SEEK_END);
					lseek(fd, 0, SEEK_SET);
					shared_file = mmap(NULL, file_size, PROT_READ, MAP_SHARED, fd, 0);

					if (shared_file == (void*) - 1) {
						sendString(error);

					} else {
						sendString(success);
					}
				}
			} else {
				sendString(error);
			}
			close(fd);

		}

		if (strncmp(request, "READ_FROM_FILE_OFFSET", size) == 0) {
			read(fd_read, &file_offset, sizeof(unsigned int));
			read(fd_read, &no_of_bytes, sizeof(unsigned int));

			sendString(request);
			if (shared_file != (void*) - 1 && shared_mem != (void*) - 1) {
				if (file_offset + no_of_bytes < file_size ) {
					memmove(shared_mem, shared_file + file_offset, no_of_bytes);
					sendString(success);
				} else {
					sendString(error);
				}
			} else {
				sendString(error);
			}
		}

		if (strncmp(request, "READ_FRON_FILE_SECTION", size) == 0) {
			read(fd_read, &section_no, sizeof(unsigned int));
			read(fd_read, &file_offset, sizeof(unsigned int));
			read(fd_read, &no_of_bytes, sizeof(unsigned int));

			sendString(request);
			if (shared_file != (void*) - 1 && shared_mem != (void*) - 1) {
				if (section_no  <= _sect_nr ) {
					memmove(shared_mem, shared_file + _sect_offset, no_of_bytes);
					sendString(success);
				} else {
					sendString(error);
				}
			} else {
				sendString(error);
			}
		}

		if (strncmp(request, "READ_FROM_LOGICAL_SPACE_OFFSET", size) == 0) {
			read(fd_read, &logical_offset, sizeof(unsigned int));
			read(fd_read, &no_of_bytes, sizeof(unsigned int));

			sendString(request);
		}
	}
	return 0;
}