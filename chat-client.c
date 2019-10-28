#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#define MAX 4096		// estava com 80
#define Socket_Adress struct sockaddr 

void func(int sockfd) { 
	char buff[MAX]; 
	int n; 

	for (;;) { 
		bzero(buff, sizeof(buff)); 
		printf("Enter the string : "); 
		n = 0; 

		while ((buff[n++] = getchar()) != '\n') 
			; 
		write(sockfd, buff, sizeof(buff)); 
		bzero(buff, sizeof(buff)); 
		read(sockfd, buff, sizeof(buff)); 
		printf("From Server : %s", buff); 

		if ((strncmp(buff, "exit", 4)) == 0) { 
			printf("Client Exit...\n"); 
			break; 
		} 
	} 
} 

int main(int argc, char** argv) { 
	int sockfd, connfd; 
	struct sockaddr_in server_adress, cli; 

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
	server_adress.sin_addr.s_addr = inet_addr(argv[1]);			// ADDR escrito nos argumentos
	server_adress.sin_port = htons(atoi(argv[2]));						// PORT escrito nos argumentos

	// connect the client socket to server socket 
	if (connect(sockfd, (Socket_Adress*)&server_adress, sizeof(server_adress)) != 0) { 
		printf("Connection with the server failed...\n"); 
		exit(0); 
	} 
	else
		printf("Connected to the server..\n"); 

	// function for chat 
	func(sockfd); 

	// close the socket 
	close(sockfd); 
} 
