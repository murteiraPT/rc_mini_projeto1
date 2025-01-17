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
#include <limits.h>
#include <math.h>

#include "packet-format.h"

#define Socket_Adress struct sockaddr

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

socklen_t servaddr_size;
struct sockaddr_in servaddr;
int last_seq_num = INT_MAX;


void send_packet(FILE* file_i, int seq_num, int sockfd) {
    data_pkt_t packet;

    fseek(file_i, 1000 * (seq_num - 1), SEEK_SET);
    memset(&packet.data, 0, sizeof(packet.data));
    int bytes = fread(packet.data, 1, 1000, file_i);

    if(bytes < 0)
        perror("fread error.");

    if(bytes < 1000 && bytes >= 0)
        last_seq_num = seq_num;
    
    packet.seq_num = seq_num;
    sendto(sockfd, (data_pkt_t *) &packet, bytes + 4, 0, (struct sockaddr *) &servaddr, servaddr_size);
}


int main(int argc, char** argv) { 
	int sockfd; 
	int port = atoi(argv[3]);
    struct hostent* host;
    int window_size = atoi(argv[4]);
        
    char* file_name = argv[1];
    host  = gethostbyname(argv[2]);
    FILE *file_i;
    file_i = fopen(file_name, "r");

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd == -1) { 
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
    int base = 1;
    ack_pkt_t ack;
    ack.selective_acks = 0;

    int next_to_send = window_size + base;

    while(ack.seq_num - 1 != last_seq_num) {

        int tentativas = 3;
        int i, j, d, e, f;
            
        // ENVIAR NUMERO WINDOW_SIZE DE PACKETS.    
        for(i = base, j = 0; i < base + window_size; i++, j++) {
            // Verificar quais os packets que sao enviados segundo o ack.selective_acks.
            if (i > last_seq_num){
                break;
            }

            if(CHECK_BIT(ack.selective_acks, j) == 0){
                send_packet(file_i, i, sockfd);
            }
        }

        for(i = base, j = 0; i < base + window_size; i++, j++) {

            // RECEBER OS ACKS DOS PACKETS ENVIADOS
            while (tentativas > 0){

                // Verificamos se houve algum timeout. Caso tenha havido, enviar de novo os que falharam.
                if(recvfrom(sockfd, (ack_pkt_t *) &ack, sizeof(ack), 0, (struct sockaddr *) &servaddr, &servaddr_size) < 0){
                    tentativas--;  

                    for(e = base, f = 0; e <= base + window_size; e++, f++) {
                        // Verificar quais os packets que sao enviados segundo o ack.selective_acks.  
                        if(CHECK_BIT(ack.selective_acks, f) == 0){
                            send_packet(file_i, e, sockfd);
                        }
                    }

                    if(tentativas == 0) {
                        fclose(file_i);
                        close(sockfd);
                        exit(-1);
                    }
                    continue;
                }

                break;
            }

            // Aqui recebeste um ack.
            tentativas = 3;


            if (ack.seq_num > base) {           
                int jumps_num = ack.seq_num - base;
                base = ack.seq_num;
                i = base;

                if (ack.seq_num > last_seq_num){
                    close(sockfd);
                    fclose(file_i);
                    return 0;
                }

                for(d = 0; d < jumps_num; d++) {
                    if(d + window_size < last_seq_num) {
                        send_packet(file_i, next_to_send, sockfd);
                        next_to_send += 1;
                    }
                }
            }
        }
    }

    fclose(file_i);
    close(sockfd);
    return 0;
} 