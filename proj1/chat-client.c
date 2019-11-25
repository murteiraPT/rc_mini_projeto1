#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h>
#include <sys/time.h>



#include <sys/types.h> 
#include <netinet/in.h> 
#include <errno.h> 
#include <arpa/inet.h> 
#include <unistd.h> 


#define MAX 4097
#define Socket_Adress struct sockaddr 
#define h_addr h_addr_list[0]


int main(int argc, char** argv) { 
	int sockfd, connfd; 
	int recv_value = 10;
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
	servaddr.sin_addr = *((struct in_addr *)newhost->h_addr);
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

		if( (select(sockfd + 1, &file_desc, NULL, NULL, NULL) < 0) && (errno!=EINTR) && recv_value != 0 ) {
			perror("Select error.");
			exit(-1);
		}

		if(FD_ISSET(0, &file_desc)) {
			memset(buffer, 0, MAX);
			
			int rd = read(0, buffer, MAX);
			
			if(rd == -1) 
				perror("Read error.");
			if(rd == 0)
				exit(0);

			if(buffer[0] == EOF)
				exit(0);
			
			if(send(sockfd, buffer, MAX, 0) == -1)
				perror("Send error.");
		}

		if(FD_ISSET(sockfd, &file_desc)) {
			memset(buffer, 0, MAX);
			
			recv_value = recv(sockfd, buffer, MAX, 0);
			if( recv_value == -1) {
				perror("Receive error.");
				exit(-1);
			}
			if(recv_value == 0){
				fprintf(stderr, "O server terminou.\n");
				close(sockfd);
				exit(0);
			}
			
			printf("%s", buffer);	
		}
	}

	// close the socket 
	close(sockfd); 
} 
