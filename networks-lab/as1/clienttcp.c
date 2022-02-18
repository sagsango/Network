#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#define MAX 80
#define PORT 8081
#define SA struct sockaddr

void func(int sockfd){
    char buff[MAX];
    int n;

    while(1){
        bzero(buff,sizeof(buff));
        printf("Enter the string: ");

        n=0;
        while((buff[n++] = getchar())!= '\n');

        write(sockfd , buff, sizeof(buff));

        bzero(buff,MAX);

        read(sockfd, buff, sizeof(buff));

        printf("\nMessage from server : %s\n", buff);

        if((strncmp("exit",buff,4)) == 0){
            printf("\n Clent exiting...\n");
        }
    }
}

int main() 
{ 
    int sockfd, connfd; 
    struct sockaddr_in servaddr, cli; 
  
    // socket create and varification 
    sockfd = socket(AF_PACKET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created..\n"); 
    bzero(&servaddr, sizeof(servaddr)); 
  
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    servaddr.sin_port = htons(8081); 
  
    // connecting the client socket to server socket 
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
        printf("connection with the server failed...\n"); 
        exit(0); 
    } 
    else
        printf("connected to the server..\n"); 
  
    // function for communicating with the server
    func(sockfd); 
  
    // close the socket 
    close(sockfd); 
} 