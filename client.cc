#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

using namespace std;

struct database_entry
{
	string st;
	int len;
}database[1000010];

int newsockfd;

void handle_requests()
{
	char buffer[256];
	uint8_t content_len;
}

int main(int argc, char *argv[])
{
	memset(database, 0, sizeof(database));
    int portno, n;
	int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
	uint8_t content_len;
	portno = atoi(getenv("SERVER_PORT"));
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) 
        perror("ERROR opening socket");
    server = gethostbyname(getenv("SERVER_ADDRESS"));
    if(server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
	newsockfd = connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr));
	if(newsockfd<0)
		perror("ERROR connecting");
	handle_requests();
}
