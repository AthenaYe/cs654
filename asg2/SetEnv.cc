#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>   
#include <cstring>
#include <cstdlib>
#include <cstdio>

using namespace std;

int main(int argc, char *argv[])
{
	if(!setenv("SERVER_PORT", argv[1], 1)&&!setenv("SERVER_ADDRESS", argv[2], 1))
	{
		puts(getenv("SERVER_PORT"));
		atoi(getenv("SERVER_PORT"));
		puts(getenv("SERVER_ADDRESS"));
		return 0;
	}
	else
	{
		puts("fail to set environment value");
		return 1;
	}
}
