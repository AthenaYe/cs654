all:
	g++ -pthread client.cc socklib.cc -o client
	g++ -pthread server.cc socklib.cc -o server
