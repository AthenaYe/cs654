#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
	uint8_t content_len;

    char buffer[256];
	char padding[1];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    
	portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
	int newsockfd = connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr));
	if(newsockfd<0)
		error("ERROR connecting");

	while(true)
	{
		printf("Please enter the message: ");
		bzero(buffer, sizeof(buffer));
		fgets(buffer, sizeof(buffer), stdin);
		content_len = (uint8_t)strlen(buffer);
		
		// Send string length
		if(send(sockfd, (char *)(&content_len), 1, 0) < 0) {
			perror("Error sending content length");
			return 1;
		}
		
		// Send string
		if(send(sockfd, buffer, (ssize_t)content_len, 0) < 0) {
			perror("ERROR writing to socket");
			return 1;
		}

		// send padding zero
		if(send(sockfd, "\0", 1, 0) < 0) {
			perror("ERROR writing to socket");
			return 1;
		}

		// recv reply msg length
		if (recv(sockfd, buffer, 1, 0) < 0) {
			perror("ERROR reading from socket");
			return 1;
		}
		ssize_t reply_len = (ssize_t)buffer[0];
		
		bzero(buffer, sizeof(buffer));
		// recv reply msg
		if (recv(sockfd, buffer, reply_len, MSG_WAITALL) < 0) {
			perror("ERROR reading from socket");
			return 1;
		}
		// recv ending zero
		if (recv(sockfd, padding, sizeof(padding), 0) < 0) {
			perror("ERROR reading from socket");
			return 1;
		}
		if (padding[0] != '\0') {
			printf("Error: reply msg should end with '\\0'\n");
		}
		printf("[%s]\n", buffer);
	}
	close(sockfd);
	return 0;
}
