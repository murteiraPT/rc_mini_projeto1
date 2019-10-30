#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#define MAX 4097
#define Socket_Adress struct sockaddr 


int main(int argc, char** argv) { 
	int sockfd, connfd; 
	struct sockaddr_in servaddr, cli;
	fd_set file_desc;
	char buffer[MAX];

	// socket create and varification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 

	if (sockfd == -1) { 
		printf("Socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created...\n"); 
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
		exit(0); 
	} 
	else
		printf("Connected to the server..\n"); 

	while(1) {

		FD_ZERO(&file_desc);
		FD_SET(0, &file_desc);
		FD_SET(sockfd, &file_desc);

		select(sockfd + 1, &file_desc, NULL, NULL, NULL);

		if(FD_ISSET(0, &file_desc)) {
			memset(buffer, 0, MAX);
			read(0, buffer, MAX);
			send(sockfd, buffer, MAX, 0);
		}

		if(FD_ISSET(sockfd, &file_desc)) {
			memset(buffer, 0, MAX);
			recv(sockfd, buffer, MAX, 0);
			printf("%s", buffer);
		}
		
	}

	// close the socket 
	close(sockfd); 
} 
