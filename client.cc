#include <time.h>       /* time */
#include "socklib.h"
#include "2pc.h" 
#include "client.h"
#include <signal.h>
#define PROB -1

using namespace std;

struct database_entry
{
	string st;
	int len;
}database[20];

struct log
{
	int index;
	string st;
}undo_log, redo_log;

void answer_commit(int sockfd)
{
	int prob  = rand() % 100;
	int len;
	if(recv(sockfd, (char *)&len, sizeof(int), 0) == -1)
	{
		perror("failed to recv data from server");
		return;
	}
	char msg[len];
	if(recv_msg(sockfd, msg, len) != 0)
	{
		perror("failed to recv server data");
		return;
	}
	printf("recv:[%s]\n", msg);
	int reply = RPLY_YES;
	if(prob <= PROB)
		reply = RPLY_NO;
	send(sockfd, (char *)&reply, sizeof(int), 0);
	stringstream ss; 
	string op_type, op_line, op_content = "";
	ss<<msg;
	ss>>op_type;
	ss>>op_line;
	if(op_type.compare("INSERT") == 0)
		ss>>op_content;
	cout<<op_type<<" "<<op_line<<" "<<op_content<<endl;
	redo_log.index = atoi(op_line.c_str());
	redo_log.st = op_content;
	undo_log.index = redo_log.index;
	undo_log.st = database[redo_log.index].st;
	return;
}

void terminate(int sockfd)
{
	puts("server ask me to terminate");
	for(int i = 0; i < 20; i++)
	{
		printf("index %d\n", i);
		cout<<"["<<database[i].st<<"]"<<endl;
	}
	exit(EXIT_SUCCESS);
	return;
}

void real_commit(int sockfd)
{
	int index = redo_log.index;
	int prob  = rand() % 100;
	int reply = COMMIT_SUCCESS;
	if(prob <= PROB)
		reply = COMMIT_FAIL;
	else if(redo_log.index == -1)
		reply == COMMIT_FAIL;
	send(sockfd, (char *)&reply, sizeof(int), 0);
	if(reply == COMMIT_FAIL)
		return;
	database[index].st = redo_log.st; 
	database[index].len = redo_log.st.size(); 
	redo_log.index = -1;
	redo_log.st = "";
	return;
}

void real_abort(int sockfd)
{

	redo_log.index = -1;
	redo_log.st = "";
	undo_log.index = -1;
	undo_log.st = "";
	return;
}

void rollback(int sockfd)
{
	int index = undo_log.index;
	database[index].st = undo_log.st;
	database[index].len = undo_log.st.size();
	real_abort(sockfd);
	return;
}

void handle_requests(int sockfd)
{
	/* initialize random seed: */
	srand (time(NULL));
	char buffer[256];
	memset(buffer, 0, sizeof(buffer));
	uint8_t content_len;
	bool ifterminate = false;
	while(true)
	{
		int type;
		if(recv(sockfd, (char *)&type, sizeof(int), 0) == -1)
		{
			perror("failed to recv data from server");
			return;
		}
		printf("msg type%d\n", type);
		switch(type)
		{
			case ASK_COMMIT: answer_commit(sockfd);
							 break;
			case MSG_COMMIT: real_commit(sockfd);
							 break;
			case MSG_ABORT: real_abort(sockfd);
							break;
			case MSG_ROLLBACK:	rollback(sockfd);
								break;
			case MSG_COMPLETE:	real_abort(sockfd);
								break;
			case MSG_TERMINATE:	terminate(sockfd);
								ifterminate = true;
								break;
			default:	break;
		}
		if(ifterminate)
		{
			close(sockfd);
			break;
		}
	}
	return;
}

int main(int argc, char *argv[])
{
	for(int i = 0; i < 20; i++)
	{
		database[i].st = "";
		database[i].len = 0;
	}
//	memset(database, 0, sizeof(database));
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
	int newsockfd = connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr));
	if(newsockfd<0)
	{
		perror("ERROR connecting");
		return 1;
	}
//	pthread_t threads_select;
//	int rc = pthread_create(&threads_select, NULL, handle_requests, (void *) (long)newsockfd);
//	if (rc) 
//	{
//		printf("ERROR; return code from pthread_create() is %d\n", rc);
//		exit(-1);
//	}
//	pthread_exit(NULL);
	handle_requests(sockfd);
	return 0;
}
