/*
	PGM file has format (for eg - P2), height and width of the Image,
	inversion factor and then a 2D array of the intensity values of the image
*/
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#define PORT 8081
#define SIZE 100


int main()
{
	struct sockaddr_in servaddr;
	FILE *upfile,*downfile;
	int i,l,counter;
	char new[SIZE];
	int w,h,in,T,t;

	//Creating socket 
	bzero((char *)&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(PORT);

	int sfd,nsfd;

	if((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Can not create Socket");
		exit(0);
	}
	else
		printf("Socket is created\n");

	//Binding the socket
	if((bind(sfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) < 0)
	{
		perror("Bind is not working");
		exit(0);
	}
	else
		printf("Socket is binded\n");

	listen(sfd, 1);char format[10];
	    

	int len = sizeof(servaddr);


	if((nsfd = accept(sfd, (struct sockaddr *)&servaddr, &len)) < 0)
	{
		perror("Accept Failed");
		exit(0);
	}
	else
		printf("Server is now accepting connection requests\n");
	
	
	//Open the file to be sent to client 
	upfile = fopen("dragon.pgm","r");
	if(upfile == NULL){
        perror("ERROR Server cant open the file");
        exit(0);
    }
	    
	//send the format of the .pgm file
	fgets(format, sizeof(format), upfile);
	write(nsfd,format,10);
	//send width of the .pgm file
	fscanf(upfile, "%d ", &w);
	T = htonl(w);
	write(nsfd,&T,sizeof(T));

	//send the height of the .pgm file
	fscanf(upfile, "%d ", &h);
	T = htonl(h);
	write(nsfd,&T,sizeof(T));

	//send the inversion factor
	fscanf(upfile, "%d ", &in);
	T = htonl(in);
	write(nsfd,&T,sizeof(T));

	//send the rest of the matrix containing the image intensity
	for(i=0; i<w*h; i++)
	{
	    fscanf(upfile, "%d ", &t);
	    T = htonl(t);
		write(nsfd,&T,sizeof(T));
    }
	//close this file
	fclose(upfile);
	printf("\nFile sent\n");
	bzero(format,10);

	//Open a new pgm file in write mode to save the inverted file
	downfile = fopen("cdragon.pgm","w");
    if(downfile == NULL)
    {
    	perror("Error. Can't write");
    	exit(0);
    }

	//Receive the format of the .pgm file and print it on the file
    read(nsfd,format,10);
    fprintf(downfile,"%s",format);

	//Receive the width of the .pgm file and print it on the file
    read(nsfd,&w,sizeof(w));
    fprintf(downfile,"%d ",ntohl(w));

	//Receive the height of the .pgm file and print it on the file
    read(nsfd,&h,sizeof(h));
    fprintf(downfile,"%d\n",ntohl(h));

	//Receive the inversion factor of the .pgm file and print it on the file
    read(nsfd,&t,sizeof(t));
    fprintf(downfile,"%d\n",ntohl(t));

    counter=0;
	//Receive the intensity values of the .pgm file and print it on the file
    for(i=0; i<ntohl(w)*ntohl(h); i++)
    {  
    	read(nsfd,&t,sizeof(t));
        counter++;
        if(counter>=50)
        {
            fprintf(downfile,"%d\n",ntohl(t));
            counter=0;
        }
        else
            fprintf(downfile,"%d ",ntohl(t));
       
    }   
	//The received file is cdragon.pgm 
	//closing the file
    fclose(downfile);
    printf("\nReceived file from client\n");

	

}