/******************************************************************************
 * * FILE: hello.c
 * * DESCRIPTION:
 * *   A "hello world" Pthreads program.  Demonstrates thread creation and
 * *   termination.
 * * AUTHOR: Blaise Barney
 * * LAST REVISED: 08/09/11
 * ******************************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define NUM_CLIENTS 2


int client_no = 0;
struct sockaddr_in serv_addr, cli_addr;

void *rw_socket(void *sock_ptr)
{
	long newsock = (long)sock_ptr;
	int newsockfd = (int)newsock;
     char buffer[256];
	 int n;
	 while(true)
	 {
		 bzero(buffer,256);
		 n = read(newsockfd,buffer,255);
		 if(n <2)
			 break;
		 if (n < 0) puts("ERROR reading from socket");
		 printf("Here is the message: %s\n",buffer);
		 n = write(newsockfd,buffer,sizeof(buffer));
		 if (n < 0) puts("ERROR writing to socket");
	 }
	 close(newsockfd);
	 pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	int rc;
	long t;
	int sockfd, portno;
	socklen_t clilen;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		puts("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = 8080;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
				sizeof(serv_addr)) < 0) 
		puts("ERROR on binding");
	listen(sockfd, NUM_CLIENTS);
	while(true)
	{
		clilen = sizeof(cli_addr);
		long newsockfd = accept(sockfd, 
				(struct sockaddr *) &cli_addr, 
				&clilen);
		if (newsockfd < 0) 
			puts("ERROR on accept");
		pthread_t threads;
		rc = pthread_create(&threads, NULL, rw_socket, (void *)newsockfd);
		if (rc){
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}
	close(sockfd);
	/* Last thing that main() should do */
	pthread_exit(NULL);
}
