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
	int newsockfd = (int)((long)sock_ptr);
	char buffer[256];
	char reply_msg[256];
	char padding[1];
	int n;
	uint8_t content_len;
	while(true)
	{
		bzero(buffer, sizeof(buffer));
		bzero(reply_msg, sizeof(buffer));
		// recv reply msg length
		n = recv(newsockfd, buffer, 1, 0);
		if (n < 0) {
			perror("ERROR reading from socket");
			break;
		}
		if (n == 0) {
			printf("Client Disconnected\n");
			break;
		}
		ssize_t msg_len = (ssize_t)buffer[0];
		// recv reply msg
		if (recv(newsockfd, buffer, msg_len, MSG_WAITALL) < 0) {
			perror("ERROR reading from socket");
			break;
		}
		// recv ending zero
		if (recv(newsockfd, padding, sizeof(padding), 0) < 0) {
			perror("ERROR reading from socket");
			break;
		}
		if (padding[0] != '\0') {
			printf("Error: reply msg should end with '\\0'\n");
			break;
		}
		
		strncpy(reply_msg, buffer, msg_len);
		content_len = (uint8_t)strlen(reply_msg);
		
		// Send string length
		if(send(newsockfd, (char *)(&content_len), 1, 0) < 0) {
			perror("Error sending content length");
			break;
		}
		
		// Send string
		if(send(newsockfd, reply_msg, (ssize_t)content_len, 0) < 0) {
			perror("ERROR writing to socket");
			break;
		}

		// send padding zero
		if(send(newsockfd, "\0", 1, 0) < 0) {
			perror("ERROR writing to socket");
			break;
		}

	}
	close(newsockfd);
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	int rc;
	long t;
	int sockfd, portno, option=1;
	socklen_t clilen;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)(&option), sizeof(option));
	if (sockfd < 0) {
		perror("ERROR opening socket");
		return 1;
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = 8080;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("ERROR on binding");
		return 1;
	}
	listen(sockfd, NUM_CLIENTS);
	while(true)
	{
		clilen = sizeof(cli_addr);
		long newsockfd = accept(sockfd, 
				(struct sockaddr *) &cli_addr, 
				&clilen);
		if (newsockfd < 0) {
			perror("ERROR on accept");
		}
		pthread_t threads;
		rc = pthread_create(&threads, NULL, rw_socket, (void *)newsockfd);
		if (rc) {
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}
	close(sockfd);
	/* Last thing that main() should do */
	pthread_exit(NULL);
	return 0;
}
