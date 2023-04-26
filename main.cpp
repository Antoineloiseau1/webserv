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

	Server	server(FAMILY, SOCKTYPE, PROTOCOLE, PORT, NBSOCKET);
	server.start();

	return 0;
}

// void connect_UsingGetAddrInfo(std::string host, unsigned short int port, int& socketfd)
// {
//     //simplified loops & error handling for concision
//     int x;

//     struct addrinfo hints, *addr;
//     //fine-tune hints according to which socket you want to open
//     hints.ai_family = FAMILY; 
//     hints.ai_socktype = SOCKTYPE; 
//     hints.ai_protocol = 0; //any protocol can be returned
//     hints.ai_flags = AI_CANONNAME | AI_ALL | AI_ADDRCONFIG;

//     //Precise here the port !
//     const char* service = std::to_string(port).c_str();
// 	const char* hostname = &host[0];
//     x =  getaddrinfo(hostname, service, &hints, &addr);
//     socketfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
//     x = connect(socketfd, addr->ai_addr, addr->ai_addrlen);
// }

// void	connect_to_server_from_client() {
// }
