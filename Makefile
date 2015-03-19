all:
	g++ -pthread client.cc -o client
	g++ -pthread server.cc -o server
