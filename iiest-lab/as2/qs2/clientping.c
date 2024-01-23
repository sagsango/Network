/*
	To calculate the RTT of communication between an UDP server and an UDP client.
*/

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <time.h>
#include <string.h>

#define PORT 8080
#define IP_ADDR "127.0.0.1"

int main(){
	int sockfd;
	int rcv_num, loop_count, len,i,optval;
	char ping = 's';
	char pong;
	struct sockaddr_in client_addr;
	struct timeval start,end;
	//Socket creation
	if((sockfd = socket(PF_INET, SOCK_DGRAM,0)) < 0){
		perror("Socket creation failed");
		exit(0);
	}
	else{
		printf("\nSocket successfully created\n");
	}
    memset(&client_addr,0,sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(PORT);
	client_addr.sin_addr.s_addr = inet_addr(IP_ADDR);
	memset(client_addr.sin_zero,'\0',sizeof(client_addr.sin_zero));
    

	printf("\nSending ping messages to server \n");
	double t1,t2;
	for( i=0 ; i<10;i++){
		if(gettimeofday(&start,NULL)){
			printf("\ntime failed\n");
			exit(0);
		}

		sendto(sockfd, &ping, sizeof(ping),
        MSG_CONFIRM, (const struct sockaddr *)&client_addr,
        sizeof(client_addr));
		optval = recvfrom(sockfd,&pong, sizeof(char),MSG_CONFIRM,
            (struct sockaddr *)&client_addr, &len);
		if(optval == -1){
			perror("Receive Error");
			exit(0);
		}

		else{
			if(gettimeofday(&end,NULL)){
				printf("\ntime failed\n");
				exit(0);
			}
			//initial value of time in ms
			t1+=start.tv_sec + (start.tv_usec/1000000.0);
			//initial recorded value of time in ms
			t2+=end.tv_sec + (end.tv_usec/1000000.0);
		}
	}

	//Calculate and print the round trip time 
	printf("\nRTT = %g ms\n",(t2-t1)/10);

	
	return(0);
}
