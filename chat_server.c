#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <sys/types.h>
#include <sys/wait.h>

#define PORT 8080 

void send_input(int sourcefd, int destfd);

int main(int argc, char **argv) {
	int server_fd; // listening socket file descriptor
	int client_1; // client socket file descriptor
    int client_2;
	struct sockaddr_in address; // address struct with ip address and port num
    int opt = 1; 
    int addrlen = sizeof(address); 

	if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("socket failed bruh");
		exit(EXIT_FAILURE);
	}

	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char *) &opt, sizeof(opt))) { 
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	if(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	if (listen(server_fd, 3) < 0) 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    }
    // 1st client enters chat room
    if((client_1 = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
    	perror("accept"); 
        exit(EXIT_FAILURE);
    }
    printf("1st client entered the chat room\n");
    if((client_2 = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept"); 
        exit(EXIT_FAILURE);
    }
    printf("2nd client entered the chat room\n");

    int status;
    int n = fork();

    if (n < 0) {
      perror("fork failed");
      exit(1);
    }

    if (n == 0) {
        send_input(client_1, client_2);
        return 0;
    }
    if (n > 0) {
        send_input(client_2, client_1);
        wait(&status);
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) == 0) {
                printf("SUCCESS\n");
            }
            else {
                printf("An error occured in validate program\n");
            }
        }
    }

    // echo client message
    /*char *buf = malloc(sizeof(char)*100);
    char *pos = buf;
    char *new_line_ptr;
    int bytes_read;
    memset(buf, 0, 200);
    int bytes_left = 199;
    printf("Client Says: ");
    while(1) {
    	bytes_read = read(new_socket, pos, bytes_left);
    	if((new_line_ptr = strchr(buf, '\n'))!=NULL) {
            // print line
    		*new_line_ptr++ = '\0';
    		if(strcmp(buf, "EXIT") ==0)
    			break;
    		printf("%s\n", buf);

            //recalculate space left in buffer 
    		bytes_left = (bytes_left - bytes_read) + strlen(buf) + 1;
    		pos = buf + strlen(new_line_ptr);

            // get rid of line just read, and shift rest to front of buffer
    		strncpy(buf, new_line_ptr, strlen(new_line_ptr));
    		memset(pos, 0, bytes_left);
    		printf("Client Says: ");
    	} else {
    		printf("%s", pos);
    		memset(pos, 0, bytes_left);
    	}
    }
*/  
    return 0;
}

void send_input(int sourcefd, int destfd) {
    int save_stdout = dup(1);
    if(dup2(destfd, 1) == -1) {
        perror("dup2 failed");
        exit(1);
    }
    char *buf = malloc(sizeof(char)*100);
    char *pos = buf;
    char *new_line_ptr;
    int bytes_read;
    memset(buf, 0, 200);
    int bytes_left = 199;
    printf("Client Says: ");
    while(1) {
        bytes_read = read(sourcefd, pos, bytes_left);
        if((new_line_ptr = strchr(buf, '\n'))!=NULL) {
            // print line
            *new_line_ptr++ = '\0';
            if(strcmp(buf, "EXIT") ==0) {
                printf("client has left from the chat\n");
                close(sourcefd);
                close(destfd);
                break;
            }
            printf("%s\n", buf);

            //recalculate space left in buffer 
            bytes_left = (bytes_left - bytes_read) + strlen(buf) + 1;
            pos = buf + strlen(new_line_ptr);

            // get rid of line just read, and shift rest to front of buffer
            strncpy(buf, new_line_ptr, strlen(new_line_ptr));
            memset(pos, 0, bytes_left);
            printf("Client Says: ");
        } else {
            printf("%s", pos);
            memset(pos, 0, bytes_left);
        }
    }
    dup2(save_stdout, 1);
}

