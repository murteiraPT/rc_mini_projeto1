#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>  
#include "packet-format.h"

int main(int argc, char const *argv[]){
	if( argc != 4)
		exit(-1);
	
	int sockfd;
	int status_packet;
	int last_seq_num = 0;
	const char* file_name = argv[1];
	int port = atoi(argv[2]);	
	//int window_size = atoi(argv[3]);
	struct sockaddr_in servaddr,cliaddr;
	data_pkt_t packet;
	ack_pkt_t ack;
  ack.selective_acks = 123;


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
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 

    // Stop-and-wait receiver 
    FILE *fp;
    fp = fopen(file_name, "w");
    for(;;){
      socklen_t len = sizeof(cliaddr);
      status_packet = recvfrom(sockfd, (struct data_pkt_t*)&packet, sizeof(packet) , 0, (struct sockaddr *) &cliaddr, &len); 
      puts("INFO DO PACKET RECEBIDO :");
      printf("%u\n", packet.seq_num);
      printf("%s\n", packet.data);
      printf("%d\n", status_packet);
  
   		
      
      if(status_packet == -1){ //Recebemos um packet corrompido
        puts("DEU MERDA");
        continue;
   		} 		

      if( packet.seq_num == last_seq_num){ //recebemos um packet que ja tinhamos recebido
        puts("JA RECEBEMOS ESTE PACKET");
        sendto(sockfd, (struct ack_pkt_t *)&ack, (size_t)sizeof(ack), 0, (struct sockaddr *) &cliaddr, sizeof(cliaddr)); 
   		}

     	else{  //tudo bem do nosso lado vamos mandar um ack 
          puts("ENTREI NO else");
          last_seq_num = packet.seq_num;
          ack.seq_num = packet.seq_num +1;
          
          fseek(fp, 1000 * (packet.seq_num - 1), SEEK_SET);
          
          fwrite(packet.data,1,sizeof(packet.data), fp);

          sendto(sockfd, (struct ack_pkt_t *)&ack, (size_t)sizeof(ack), 0, ( struct sockaddr *) &cliaddr, sizeof(cliaddr));     
     	}

      if(strlen(packet.data) < 1000){
        fclose(fp);
        puts("ULTIMO PACKET ACABOU");
        exit(-1 );
      }
    
    }
	return 0;
}
