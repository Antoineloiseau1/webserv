#include "Networking/Server.hpp"
#include "parsing/parsing.hpp"
#include "Networking/Server.hpp"

#define FAMILY AF_INET
#define	SOCKTYPE SOCK_STREAM
#define PROTOCOLE 0

void	manage_events() {
//use poll or kqueue
}



int main(int argc, char *argv[], char *envp[]) {
	if(argc != 2) {
		std::cerr << "usage: ./webserv <config_file>" << std::endl; //should have a default config file so no error here
		exit(EXIT_FAILURE);
	}

	data data(static_cast<std::string>(argv[1]));
	if (data.getServers().empty())
		return 1;

	std::vector<int> ports = data.getPorts();

	Server server(FAMILY, SOCKTYPE, PROTOCOLE, ports, data.getPortsNbr(), envp, data); //leaaaaaaaaaaaaks
	server.start();

	return 0;
}

