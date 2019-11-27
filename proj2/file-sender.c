#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h> 
#include <netinet/in.h> 
#include <errno.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <stddef.h>


#include "packet-format.h"

#define Socket_Adress struct sockaddr
#define SEGMENTS 1000


data_pkt_t get_chunk(char* file_name, uint32_t seq_num, FILE * file_i) {
    data_pkt_t packet;
      
    fseek(file_i, 1000 * (seq_num - 1), SEEK_SET);
    memset(&packet.data,0,sizeof(packet.data));
    int bytes = fread(packet.data, 1000, 1, file_i);

    if(bytes < 0)
        perror("fread error.");
 
    packet.seq_num = seq_num; 

    return packet;
}


int main(int argc, char** argv) { 
	int sockfd; 
    uint32_t seq_num = 1;
	int port = atoi(argv[3]);
	struct sockaddr_in servaddr;
    struct hostent* host;
    socklen_t servaddr_size;
        
    char* file_name = argv[1];
    host  = gethostbyname(argv[2]);
    FILE *file_i;
    file_i = fopen(file_name, "r");

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd == -1) { 
		printf("Socket creation failed...\n"); 
		exit(-1); 
	} 

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval)) < 0) {
        perror("Socket option failed...\n"); 
		exit(-1); 
    }

	bzero(&servaddr, sizeof(servaddr)); 

	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr = *((struct in_addr *)host->h_addr);
	servaddr.sin_port = htons(port);
    
    servaddr_size = sizeof(servaddr);
    int bytes = 1000;

    while(bytes >=1000) {

        int tentativas = 3;

        data_pkt_t packet;
        ack_pkt_t ack;



        fseek(file_i, 1000 * (seq_num - 1), SEEK_SET);
        memset(&packet.data,0,sizeof(packet.data));
        bytes = fread(packet.data, 1, 1000, file_i);

        if(bytes < 0)
            perror("fread error.");
 
        packet.seq_num = seq_num; 


        printf("bytes = %d\n", bytes);
        printf("seq_sum%u\n", packet.seq_num);
        printf("data: %s\n", packet.data);
        

        sendto(sockfd, (data_pkt_t *) &packet, bytes + offsetof(data_pkt_t, data), 0, (struct sockaddr *) &servaddr, servaddr_size);

        while (tentativas > 0){

            if(recvfrom(sockfd, (ack_pkt_t *) &ack, sizeof(ack), 0, (struct sockaddr *) &servaddr, &servaddr_size) < 0){
                sendto(sockfd, (data_pkt_t *) &packet, bytes + offsetof(data_pkt_t, data), 0, (struct sockaddr *) &servaddr, servaddr_size);
                tentativas--;
    
            printf("ACK : %d\n", ack.seq_num);

                if(tentativas == 0) {
                    fclose(file_i);
                    close(sockfd);
                    exit(-1);
                }
                continue;
            }
            break;
        }

        seq_num++;

    }

    fclose(file_i);
    close(sockfd);
    return 0;
} 