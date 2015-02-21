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
#include <vector>
#include <iostream>
#include <unistd.h>

using namespace std;

vector<string> buff;

void *ReadString(void *threadid)
{
	long tid;
	tid = (long)threadid;
	printf("Hello World! It's me, thread #%ld!\n", tid);
	while(true)
	{
		string st;
		cin>>st;
		buff.push_back(st);
	}
	pthread_exit(NULL);
}

void *SendString(void *threadid)
{
	long tid;
	tid = (long)threadid;
	printf("Hello World! It's me, thread #%ld!\n", tid);
	while(true)
	{
		if(buff.empty())
			continue;
		string st = buff.front();
		buff.erase(buff.begin());
		cout<<"puts"<<st<<endl;
		sleep(5);
	}
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
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

	/* Last thing that main() should do */
	pthread_exit(NULL);
}
