#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/time.h>
#define MAX 4097
#define Socket_Adress struct sockaddr 


int main(int argc, char** argv) { 
	int sockfd, connfd; 
	char buffer[MAX];
	struct sockaddr_in servaddr, cli;
	fd_set file_desc;

	// socket create and varification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 

	if (sockfd == -1) { 
		printf("Socket creation failed...\n"); 
		exit(-1); 
	} 
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 

	struct hostent * newhost = gethostbyname(argv[1]);

	int port = atoi(argv[2]);
	
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr = *((struct in_addr *)newhost->h_addr) ; 
	servaddr.sin_port = htons(port);

	// connect the client socket to server socket 
	if (connect(sockfd, (Socket_Adress*)&servaddr, sizeof(servaddr)) != 0) { 
		printf("Connection with the server failed...\n"); 
		exit(-1); 
	} 
	while(1) {

		FD_ZERO(&file_desc);
		FD_SET(0, &file_desc);
		FD_SET(sockfd, &file_desc);

		int sel = select(sockfd + 1, &file_desc, NULL, NULL, NULL);

		if (sel == -1) {
			perror("Select error.");
			exit(-1);
		}

		if(FD_ISSET(0, &file_desc)) {
			memset(buffer, 0, MAX);
			
			if(read(0, buffer, MAX) == -1) {
				perror("Read error.");
			}
			
			if(send(sockfd, buffer, MAX, 0) != strlen(buffer)) {
				perror("Send error.");
				exit(-1);
			}

		} else {
			perror("FD_ISSET error.");
			exit(-1);
		}

		if(FD_ISSET(sockfd, &file_desc)) {
			memset(buffer, 0, MAX);
			
			if(recv(sockfd, buffer, MAX, 0) == -1) {
				perror("Receive error.");
				exit(-1);
			}
			
			printf("%s", buffer);
			
		} else {
			perror("FD_ISSET error.");
			exit(-1);
		}
		
	}

	// close the socket 
	close(sockfd); 
} 
