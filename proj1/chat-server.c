#include <stdio.h> 
#include <sys/types.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <string.h>
#include <errno.h> 
#include <arpa/inet.h> 
#include <sys/time.h> 
#include <unistd.h> 

#define MAX_MESSAGE 4097
#define MAX_CLIENTS 1000

int main(int argc , char *argv[]) { 
    int i, opt = 1; 
    int main_socket, addrsize , new_client_socket , client_socket[MAX_CLIENTS], read_out_value; 
    int main_socket_copy, socket_descriptor; 
    
    char buffer[MAX_MESSAGE];  
    char message[MAX_MESSAGE + 17];         
    
    struct sockaddr_in address; 
    fd_set readfds; 
            
    for (i = 0; i < MAX_CLIENTS; i++) 
        client_socket[i] = 0; 
 
    if((main_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0){ 
        perror("ERROR -> socket"); 
        exit(1); 
    } 

    if(setsockopt(main_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0){ 
        perror("ERROR -> setsockopt"); 
        exit(1); 
    } 
    
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons(atoi(argv[1])); 
        
    if (bind(main_socket, (struct sockaddr *)&address, sizeof(address)) < 0){ 
        perror("ERROR -> bind"); 
        exit(1); 
    } 
    printf("Listening on port: %s \n", argv[1]); 
        
    if (listen(main_socket, 3) < 0){ 
        perror("ERROR -> listen"); 
        exit(1); 
    } 
        
    addrsize = sizeof(address); 
        
    while(1){ 
        FD_ZERO(&readfds); 
        FD_SET(main_socket, &readfds); 
        main_socket_copy = main_socket; 
    
        for(i = 0; i < MAX_CLIENTS; i++){ 
            socket_descriptor = client_socket[i];                 
            
            if(socket_descriptor > 0) 
                FD_SET( socket_descriptor , &readfds);                 

            if(socket_descriptor > main_socket_copy) 
                main_socket_copy = socket_descriptor; 
        } 
    
         

        if((select( main_socket_copy + 1 , &readfds , NULL , NULL , NULL) < 0) && (errno!=EINTR)) 
            printf("ERROR -> select"); 

            
        if(FD_ISSET(main_socket, &readfds)){ 
          
            if((new_client_socket = accept(main_socket, (struct sockaddr *)&address, (socklen_t*)&addrsize))<0){ 
                perror("ERROR -> accept"); 
                exit(1); 
            } 
            
            memset(buffer, 0, MAX_MESSAGE);
            sprintf(buffer,"%s:%d joined.\n", inet_ntoa(address.sin_addr) , ntohs(address.sin_port)); 
            printf("%s",buffer);

            for(i = 0; i < MAX_CLIENTS; i++) { 
                if(client_socket[i] == 0){ 
                    client_socket[i] = new_client_socket; 
                    break; 
                } 
            }

            for(i = 0; i < MAX_CLIENTS; i++)  
                if(client_socket[i] != 0)
                    if(send(client_socket[i], buffer, MAX_MESSAGE, 0) == -1)
                        perror("ERROR -> send");
        } 
            
        for(i = 0; i < MAX_CLIENTS; i++){ 
            socket_descriptor = client_socket[i]; 
            memset(buffer, 0, MAX_MESSAGE);
            memset(message, 0, MAX_MESSAGE);
            
            if(FD_ISSET(socket_descriptor, &readfds)){ 
                getpeername(socket_descriptor, (struct sockaddr*)&address, (socklen_t*)&addrsize); 

                if ((read_out_value = read(socket_descriptor, buffer, MAX_MESSAGE)) == 0){ 
                    sprintf(message, "%s:%d left.\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                    printf("%s", message); 

                    for (int j = 0; j < MAX_CLIENTS; j++)
                        if(client_socket[j] != 0 && client_socket[j] != client_socket[i])
                            send(client_socket[j], message, MAX_MESSAGE, 0);  
                        
                    
                    close(socket_descriptor); 
                    client_socket[i] = 0; 
                } 
                    
                else{ 
                    buffer[read_out_value] = '\0'; 
                    sprintf(message,"%s:%d %s",inet_ntoa(address.sin_addr),ntohs(address.sin_port),buffer);
                    printf("%s", message);
                    
                    for (int j = 0; j < MAX_CLIENTS; j++)
                        if(client_socket[j] != 0 && client_socket[j] != client_socket[i])
                            if(send(client_socket[j], message, MAX_MESSAGE, 0) == -1)
                                perror("ERROR -> send"); 
                        
                    
                } 
            } 
        } 
    }       
    return 0; 
} 
