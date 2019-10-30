#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#define MAX 4097
#define Socket_Adress struct sockaddr 


int main(int argc, char** argv) { 
	int sockfd, connfd; 
	struct sockaddr_in server_adress, cli;
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
	bzero(&server_adress, sizeof(server_adress)); 

	// assign IP, PORT 
	server_adress.sin_family = AF_INET; 
	server_adress.sin_addr.s_addr = inet_addr(argv[1]);					// ADDR escrito nos argumentos
	server_adress.sin_port = htons(atoi(argv[2]));						// PORT escrito nos argumentos

	// connect the client socket to server socket 
	if (connect(sockfd, (Socket_Adress*)&server_adress, sizeof(server_adress)) != 0) { 
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
			i = 0;
			memset(buffer, 0, MAX);
			recv(sockfd, buffer, MAX, 0);

			while(buffer[i] != "\0") {
				printf("%s", buffer);
				i++;
			}
			print("\0");
		}		
	}
	// close the socket 
	close(sockfd); 
} 
