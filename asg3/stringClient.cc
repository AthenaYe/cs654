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

vector<string> buff;
int sockfd;

void *ReadString(void *threadid)
{
	long tid;
	tid = (long)threadid;
	while(true)
	{
		string st;
		getline(cin, st);
		buff.push_back(st);
	}
	pthread_exit(NULL);
}

void *SendString(void *threadid)
{
	long tid;
	char padding[1];
	char buffer[256];
	uint8_t content_len;
	tid = (long)threadid;
	while(true)
	{
		if(buff.empty())
			continue;
		string st = buff.front();
		buff.erase(buff.begin());
		bzero(buffer, sizeof(buffer));
		strcpy(buffer, st.c_str());
		content_len = (uint8_t)strlen(buffer);
		
		// Send string length
		if(send(sockfd, (char *)(&content_len), 1, 0) < 0) {
			perror("Error sending content length");
			return NULL;
		}
		
		// Send string
		if(send(sockfd, buffer, (ssize_t)content_len, 0) < 0) {
			perror("ERROR writing to socket");
			return NULL;
		}

		// send padding zero
		if(send(sockfd, "\0", 1, 0) < 0) {
			perror("ERROR writing to socket");
			return NULL;
		}

		// recv reply msg length
		if (recv(sockfd, buffer, 1, 0) < 0) {
			perror("ERROR reading from socket");
			return NULL;
		}
		ssize_t reply_len = (ssize_t)buffer[0];
		
		bzero(buffer, sizeof(buffer));
		// recv reply msg
		if (recv(sockfd, buffer, reply_len, MSG_WAITALL) < 0) {
			perror("ERROR reading from socket");
			return NULL;
		}
		// recv ending zero
		if (recv(sockfd, padding, sizeof(padding), 0) < 0) {
			perror("ERROR reading from socket");
			return NULL;
		}
		if (padding[0] != '\0') {
			perror("Error: reply msg should end with '\\0'\n");
		}
		printf("Server: %s\n", buffer);
		sleep(2);
	}
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
	uint8_t content_len;
	portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        perror("ERROR opening socket");
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
		perror("ERROR connecting");
	/*
	 * open thread section
	 */
	pthread_t read_th, sock_th;
	int rc;
	long t;
	for(t = 0; t < 2; t++)
	{
		if(t == 0)
			rc = pthread_create(&read_th, NULL, ReadString, (void *)t);
		else if (t == 1)
			rc = pthread_create(&sock_th, NULL, SendString, (void *)t);
		if (rc){
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}
	pthread_exit(NULL);
}
