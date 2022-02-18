#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#define PORT 8081
#define SIZE 100

int main(int argc, char * argv[])
{

	FILE *fp;
	struct sockaddr_in servaddr;
	char str[SIZE];
	int sfd,len;
    int t,h,w,intensity,T,counter,i;
    char format[10];
    FILE *finalimage;

	bzero((char *)&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(PORT);

    //create socket
	if((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Can not create Socket");
		exit(0);
	}
	else
		printf("Socket is created\n");

	//Try to connect to the server
    if((connect(sfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) < 0)
	{
		perror("Connect is not working");
		exit(0);
	}
	else
		printf("Connecting...\n");
	 


    bzero(format,10);

    //Open a file in write mode to save the image sent by the server
    finalimage = fopen("inverted.pgm","w");
    if(finalimage == NULL)
    {
    	perror("Error. Can't write");
    	exit(0);
    }

    //Receive the format, width, height and the inversion factor of the .pgm file
    read(sfd,format,10);
    fprintf(finalimage,"%s",format);

    read(sfd,&w,sizeof(w));
    fprintf(finalimage,"%d ",ntohl(w));

    read(sfd,&h,sizeof(h));
    fprintf(finalimage,"%d\n",ntohl(h));

    read(sfd,&t,sizeof(t));
    fprintf(finalimage,"%d\n",ntohl(t));

    counter=0;

    //Receive the intensity of the .pgm file being sent by the server
    for(i=0; i<ntohl(w)*ntohl(h); i++)
    {  
    	read(sfd,&t,sizeof(t));
        //As this code receives the intensities of the .pgm, invert them by 255 
        fprintf(finalimage,"%d ",255-ntohl(t));
       
    }   
    fclose(finalimage);
    printf("\nImage Downloaded\n");


    finalimage = fopen("inverted.pgm","r");

    if(finalimage == NULL)
    {
        perror("Client cant open file");
        exit(0);
    }

    //Send the inverted image back to the server
    bzero(format,10);
    fgets(format, sizeof(format), finalimage);
    write(sfd,format,10);

    fscanf(finalimage, "%d ", &h);
    T= htonl(h);
    write(sfd,&T,sizeof(T));

    fscanf(finalimage, "%d ", &w);

    T= htonl(w);
    write(sfd,&T,sizeof(T));

    fscanf(finalimage, "%d ", &intensity);
    T = htonl(intensity);
    write(sfd,&T,sizeof(T));

    for( i=0; i<w*h; i++)
    {
        fscanf(finalimage, "%d ", &t);
        T = htonl(intensity-t);
        write(sfd,&T,sizeof(T));
    }
    fclose(finalimage);
    printf("\nInverted Image File Sent\n");

}