#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <signal.h>

#define PORT 8080 

int main(int argc, char **argv) {

	struct sockaddr_in serv_addr;
	int sock = 0;

	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Socket Creation Error");
		exit(EXIT_FAILURE);
	}
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
		perror("Address not supported");
		exit(EXIT_FAILURE);
	}
	if(connect(sock, (const struct sockaddr *) &serv_addr, sizeof(serv_addr))) {
		perror("connection failed");
		exit(EXIT_FAILURE);
	}

	// no need for pipe here? just use a kill signal..

	int n = fork();

	if (n < 0) {
      perror("fork failed");
      exit(1);
    }
    //recieving
	if(n==0) {
		char *new_messages = malloc(sizeof(char)*200);
		memset(new_messages, 0, 200);
		while(1) {
			read(sock, new_messages, 199);
			printf("%s", new_messages);
			memset(new_messages, 0, 199);
		}
	}
	//shipping
	if (n > 0) {

		char *buf = malloc(sizeof(char)*100);
		memset(buf, 0, 100);
		while(1) {
			read(0, buf, 100);
			if(strcmp(buf, "EXIT\n") == 0) {
				write(sock, buf, 100);
				int ret;
				if((ret = kill(n, SIGQUIT)) == -1) {
					perror("Did not kill client reading process");
					exit(EXIT_FAILURE);
				}
				break;
			}
			write(sock, buf, 100);
			memset(buf, 0, 100);
		}
	}
	close(sock);
	return 0;
}