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
#include <map>

#define NUM_CLIENTS 5
#define hostname string
#define fd int

using namespace std;

struct sockaddr_in serv_addr, cli_addr;
map<hostname, fd> client_hostname_set;
map<fd, hostname> client_fd_set;