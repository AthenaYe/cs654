#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/select.h>
#include <set>
#include <sstream>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>

#define NUM_CLIENTS 5

using namespace std;


struct sockaddr_in serv_addr, cli_addr;
fd_set rfds;
set<int> rfds_set;
int max_fd;

string StringProcess(char buffer[])
{
	stringstream ss;
	ss << buffer;
	string st;
	string ret;
	while(ss>>st)
	{
		st[0] = toupper(st[0]);
		ret += st + " ";
	}
	return ret;
}

void rw_socket(int newsockfd)
{
	char buffer[256];
	char reply_msg[256];
	char padding[1];
	int n;
	uint8_t content_len;
	bzero(buffer, sizeof(buffer));
	bzero(reply_msg, sizeof(buffer));
	// recv reply msg length
	n = recv(newsockfd, buffer, 1, 0);
	if (n < 0) {
		perror("ERROR reading from socket");
		rfds_set.erase(newsockfd);
		return;
	}
	if (n == 0) {
		printf("Client Disconnected\n");
		FD_CLR(newsockfd, &rfds);
		rfds_set.erase(newsockfd);
		return;
	}
	ssize_t msg_len = (ssize_t)buffer[0];
	// recv reply msg
	if (recv(newsockfd, buffer, msg_len, MSG_WAITALL) < 0) {
		perror("ERROR reading from socket");
		FD_CLR(newsockfd, &rfds);
		rfds_set.erase(newsockfd);
		return;
	}
	printf("msg recv: %s\n", buffer);
	// recv ending zero
	if (recv(newsockfd, padding, sizeof(padding), 0) < 0) {
		perror("ERROR reading from socket");
		FD_CLR(newsockfd, &rfds);
		rfds_set.erase(newsockfd);
		return;
	}
	if (padding[0] != '\0') {
		printf("Error: reply msg should end with '\\0'\n");
		FD_CLR(newsockfd, &rfds);
		rfds_set.erase(newsockfd);
		return;
	}
	string st = StringProcess(buffer);
	strncpy(reply_msg, st.c_str(), msg_len);
	content_len = (uint8_t)strlen(reply_msg);

	// Send string length
	if(send(newsockfd, (char *)(&content_len), 1, 0) < 0) {
		perror("Error sending content length");
		FD_CLR(newsockfd, &rfds);
		rfds_set.erase(newsockfd);
		return;
	}

	// Send string
	if(send(newsockfd, reply_msg, (ssize_t)content_len, 0) < 0) {
		perror("ERROR writing to socket");
		FD_CLR(newsockfd, &rfds);
		rfds_set.erase(newsockfd);
		return;
	}

	// send padding zero
	if(send(newsockfd, "\0", 1, 0) < 0) {
		perror("ERROR writing to socket");
		FD_CLR(newsockfd, &rfds);
		rfds_set.erase(newsockfd);
		return;
	}
	return;
}

void init_rfds(void)
{
	set<int>::iterator it;
	for(it = rfds_set.begin(); it != rfds_set.end(); it++)
	{
		FD_SET(*it, &rfds);
	}
}

void *select_client(void *sockfd_ptr)
{
	struct timeval timeout;

	/* Set time limit. */
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	while(true)
	{
		init_rfds();
		int retval = select(max_fd+1, &rfds, NULL, NULL, &timeout);
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		if (retval == -1)
			perror("select() error");
		else if (retval)
		{
//			printf("Data is available now.\n");
			set<int>::iterator it;
			for(it = rfds_set.begin(); it != rfds_set.end(); it++)
			{
				if(FD_ISSET(*it, &rfds))
				{
					rw_socket(*it);
				}
			}
		}
		else
		{
		//	printf("No data within %ld seconds.\n", timeout.tv_sec);
		}
		FD_ZERO(&rfds);
	}
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	int rc;
	long t;
	int sockfd, option=1;
	char hostname[256];

	socklen_t clilen;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)(&option), sizeof(option));
	if (sockfd < 0) {
		perror("ERROR opening socket");
		return 1;
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = 0;
	
	socklen_t sockaddr_len = (socklen_t)sizeof(struct sockaddr);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sockaddr_len) < 0) {
		perror("ERROR on binding");
		return 1;
	}
	
	// Print Server Socket Info
	gethostname(hostname, sizeof(hostname));
	getsockname(sockfd, (struct sockaddr *)&serv_addr, &sockaddr_len);
	printf("SERVER_ADDRESS %s\n", hostname);
	printf("SERVER_PORT %d\n", ntohs(serv_addr.sin_port));
	
	listen(sockfd, NUM_CLIENTS);
	max_fd = sockfd;
	/*
	 * set file descriptor set
	 */

	FD_ZERO(&rfds);
	pthread_t threads_select;
	rc = pthread_create(&threads_select, NULL, select_client, (void *) (long)sockfd);
	if (rc) {
		printf("ERROR; return code from pthread_create() is %d\n", rc);
		exit(-1);
	}
	while(true)
	{
		clilen = sizeof(cli_addr);
		int newsockfd = accept(sockfd, 
				(struct sockaddr *) &cli_addr, 
				&clilen);
		if (newsockfd < 0) {
			perror("ERROR on accept");
		}
		max_fd = max(max_fd, newsockfd);
		rfds_set.insert(newsockfd);
	}
	close(sockfd);
	pthread_exit(NULL);
	return 0;
}
