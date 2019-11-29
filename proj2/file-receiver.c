#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>  
#include "packet-format.h"

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))


int changeSelective(int selective_acks, int base, int received_packet){
  int shift = received_packet - base;
  return selective_acks | ( 1 << shift ); 
}

int main(int argc, char const *argv[]){
	
  if( argc != 4)
		exit(-1);
	
	int sockfd;
	int status_packet;
  int last_packet_seq_num = -1;
	const char* file_name = argv[1];
	int port = atoi(argv[2]);	
	struct sockaddr_in servaddr,cliaddr;
	data_pkt_t packet;
  ack_pkt_t ack;

  // Creating socket file descriptor 
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){ 
      perror("socket creation failed"); 
      exit(-1); 
  } 
    
  memset(&servaddr, 0, sizeof(servaddr));  
  memset(&cliaddr, 0, sizeof(cliaddr)); 
  
  // Filling server information 
  servaddr.sin_family = AF_INET; // IPv4 
  servaddr.sin_addr.s_addr = INADDR_ANY; 
  servaddr.sin_port = htons(port); 
    
  // Bind the socket with the server address 
  if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){ 
      perror("bind failed"); 
      exit(EXIT_FAILURE); 
  } 

  FILE *fp;
  fp = fopen(file_name, "w");
  int base = 1;
	int window_size = atoi(argv[3]);
  ack.seq_num = base;
  ack.selective_acks = 0;



 for(;;){
    socklen_t len = sizeof(cliaddr);
    status_packet = recvfrom(sockfd, (struct data_pkt_t*)&packet, sizeof(packet) , 0, (struct sockaddr *) &cliaddr, &len); 
    
    if(status_packet == -1){ //Recebemos um packet corrompido
      continue;
    } 		

    if(status_packet < 1004){
      last_packet_seq_num = packet.seq_num;
    }


    if(packet.seq_num < base || packet.seq_num > base + window_size){ //recebemos o packet da base podemos avançar a nossa window
        sendto(sockfd, (struct ack_pkt_t *)&ack, (size_t)sizeof(ack), 0, (struct sockaddr *) &cliaddr, sizeof(cliaddr));     
    }

    else{  
        
        if(packet.seq_num != base){
          ack.selective_acks = (changeSelective(ack.selective_acks, base, packet.seq_num));
    
          fseek(fp, 1000 * (packet.seq_num - 1), SEEK_SET);
          fwrite(packet.data, 1, status_packet - 4, fp);
          sendto(sockfd, (struct ack_pkt_t *)&ack, (size_t)sizeof(ack), 0, ( struct sockaddr *) &cliaddr, sizeof(cliaddr));     

        }

        if(packet.seq_num == base){
          fseek(fp, 1000 * (packet.seq_num - 1), SEEK_SET);
          fwrite(packet.data, 1, status_packet - 4, fp);
          
          while(CHECK_BIT(ack.selective_acks, 0) != 0){
            base++;
            ack.selective_acks >>= 1;
          }

          base++;
          ack.selective_acks >>= 1;
         
          ack.seq_num = base;
          sendto(sockfd, (struct ack_pkt_t *)&ack, (size_t)sizeof(ack), 0, ( struct sockaddr *) &cliaddr, sizeof(cliaddr));     
  
        }        
    }

    if(base == last_packet_seq_num + 1){
      fclose(fp);
      close(sockfd);
      exit(0);
    }
  }
	return 0;
}
