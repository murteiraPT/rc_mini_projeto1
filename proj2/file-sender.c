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

#include "packet-format.h"

#define Socket_Adress struct sockaddr
#define SEGMENTS 1000


data_pkt_t get_chunk(char* file_name, uint32_t seq_num, FILE * file_i) {
    int i;
    data_pkt_t packet;


    fseek(file_i, 1000 * (seq_num - 1), SEEK_SET);
    fgets(packet.data, 1000, file_i);
    packet.seq_num = seq_num;   
    return packet;
}


int main(int argc, char** argv) { 
	int sockfd; 
    uint32_t seq_num = 1;
	int port = atoi(argv[3]);
	struct sockaddr_in servaddr;
	struct hostent * newhost = gethostbyname(argv[2]);
    char* file_name = argv[1];
    
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
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(port);
    
    while(1) {

        int tentativas = 3;
        clock_t t;
        data_pkt_t packet = get_chunk(file_name, seq_num, file_i);
        ack_pkt_t ack;

        printf("%u\n", packet.seq_num);
        printf("%s\n", packet.data);
        

        sendto(sockfd, (data_pkt_t *) &packet, sizeof(data_pkt_t), MSG_CONFIRM, (struct sockaddr *) &servaddr, sizeof(servaddr));

        while (tentativas > 0){

            if(recvfrom(sockfd, (ack_pkt_t *) &ack, sizeof(ack_pkt_t), 0, (struct sockaddr *) &servaddr,(socklen_t *) sizeof(servaddr)) < 0){
                sendto(sockfd, (data_pkt_t *) &packet, sizeof(data_pkt_t), MSG_CONFIRM, ( struct sockaddr *) &servaddr, sizeof(servaddr));
                tentativas--;

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

        if(strlen(packet.data) < 1000) {
            fclose(file_i);
	        close(sockfd); 
        }
    }
} 