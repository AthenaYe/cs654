all:
	g++ -lpthread stringClient.cc -o stringClient
	g++ -lpthread stringServer.cc -o stringServer
