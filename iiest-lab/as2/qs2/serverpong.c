/*
	To calculate the RTT of communication between an UDP server and an UDP client.
*/

#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>

#define PORT 8080
#define IP_ADDRESS "127.0.0.1"

int main(){
	int sockfd,sockfd2;
	int rcv_num, counter, i;
	char ping;
	struct sockaddr_in server_addr,client_addr;
	
	int sin_size = sizeof(client_addr), k, num_packet, optval;
	
	//Open socket 
	if((sockfd = socket(PF_INET,SOCK_DGRAM,0))== -1){
		perror("\nsocket error");
		exit(0);
	}

    	memset(&server_addr,0,sizeof(server_addr));
    	memset(&client_addr,0,sizeof(client_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
	memset(server_addr.sin_zero,'\0',sizeof(server_addr.sin_zero));

	//allow the reuse of the port
	optval = 1;
	if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(int))==-1){
		perror("setsocketopt error");
		exit(0);
	}
	else {
		printf("\nReusable port\n");
	}
	//bind address
	if(bind(sockfd,(struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		perror("binding failed");
		exit(0);
	}
	printf("\nBinding successfull\n");

	
	printf("\nSending back 10 pong messages\n");
	for(i = 0;i<10;i++){
		optval = recvfrom(sockfd,&ping,sizeof(ping),MSG_WAITALL,
        (struct sockaddr *)&client_addr,&sin_size);
		if(optval!=0){
			//send pong message
			sendto(sockfd,&ping,sizeof(ping),MSG_CONFIRM,
            (const struct sockaddr *)&client_addr,sin_size);
		}

		else{
			perror("Receive error");
			exit(0);
		}
	}
	printf("\nClosing socket to inhibit further communication\n");
	shutdown(sockfd,2);

	return 0;
}
