#include "Networking/Server.hpp"

#define PORT 80
#define FAMILY AF_INET
#define	SOCKTYPE SOCK_STREAM
#define PROTOCOLE 0
#define NBSOCKET 1

void	manage_events() {
//use poll or kqueue
}

int main(int argc, char *argv[], char *envp[]) {
	if(argc != 2) {
		std::cerr << "usage: ./webserv <config_file>" << std::endl;
		exit(EXIT_FAILURE);
	}
	(void)argv;
	(void)envp;
	int ports[2] = {PORT, 4242};
	Server	server(FAMILY, SOCKTYPE, PROTOCOLE, ports, NBSOCKET);
	server.start();

	return 0;
}

