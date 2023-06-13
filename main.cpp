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

	std::string config_file;

	if(argc != 2) {
		config_file = "configuration/test_conf"; //set to default later
	}
	else
		config_file = argv[1];

	data data(config_file);
	if (data.getServers().empty())
		return 1;

	std::vector<int> ports = data.getPorts();

	Server server(FAMILY, SOCKTYPE, PROTOCOLE, ports, data.getPortsNbr(), envp, data);
	server.start();

	return 0;
}

