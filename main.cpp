#include "Networking/Server.hpp"
#include "parsing/parsing.hpp"

#define FAMILY AF_INET
#define	SOCKTYPE SOCK_STREAM
#define PROTOCOLE 0

void	manage_events() {
//use poll or kqueue
}



int main(int argc, char *argv[], char *envp[]) {
	if(argc != 2) {
		std::cerr << "usage: ./webserv <config_file>" << std::endl;
		exit(EXIT_FAILURE);
	}

	data data(static_cast<std::string>(argv[1]));
	if (data.getData().empty())
		return 1;

	//future loop for each server
	int *ports = data.getPorts();
	Server	server(FAMILY, SOCKTYPE, PROTOCOLE, ports, data.getPortsNbr(), envp, data);
	
	server.start();

	return 0;
}

