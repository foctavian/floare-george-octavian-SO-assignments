#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define PIPE_write "RESP_PIPE_55635"
#define PIPE_read "REQ_PIPE_55635"

int main(int argc, char** argv){

	int fd_write = -1;
	int fd_read = -1;

	mkfifo(PIPE_write, 0600);
		
	fd_read = open(PIPE_read, O_RDONLY);
	if(fd_read == -1){
		printf("ERROR\ncannot open the request pipe");
		return 1;
	}
	
	fd_write = open(PIPE_write, O_WRONLY);
	if(fd_write == -1){
		perror("ERROR\ncannot create the response pipe");
		return 1;
	}

	char begin[] = "BEGIN";
	char size = strlen(begin);
	write(fd_write, &size, sizeof(char));
	for(int i=0;i<strlen(begin);i++){
		write(fd_write, &begin[i], sizeof(char));

	}
	printf("SUCCESS");
	

	close(fd_write);
	close(fd_read);
	unlink(PIPE_write);
	return 0;
}