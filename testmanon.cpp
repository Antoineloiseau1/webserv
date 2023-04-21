#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <netdb.h>

#define PORT 8080
#define FAMILY AF_INET
#define	SOCKTYPE SOCK_STREAM


void connect_UsingGetAddrInfo(std::string host, unsigned short int port, int& socketfd)
{
    //simplified loops & error handling for concision
    int x;

    struct addrinfo hints, *addr;
    //fine-tune hints according to which socket you want to open
    hints.ai_family = FAMILY; 
    hints.ai_socktype = SOCKTYPE; 
    hints.ai_protocol = 0; //any protocol can be returned
    hints.ai_flags = AI_CANONNAME | AI_ALL | AI_ADDRCONFIG;

    //Precise here the port !
    const char* service = std::to_string(port).c_str();
	const char* hostname = &host[0];
    x =  getaddrinfo(hostname, service, &hints, &addr);
    socketfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    x = connect(socketfd, addr->ai_addr, addr->ai_addrlen);
}

void	connect_to_server_from_client() {
}


int main(void) {

	int server_fd = socket(PF_INET, SOCKTYPE, 0);
	struct linger sl;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &sl, sizeof(int));
	if (server_fd == -1)
	{
		std::cerr << strerror(errno);
		return (1);
	}
	struct sockaddr_in	address;
	address.sin_family = FAMILY;
	address.sin_port = htons(PORT);
	address.sin_addr.s_addr = INADDR_ANY;

 	memset(address.sin_zero, '\0', sizeof address.sin_zero);

	int addrlen = sizeof(address);

	if (bind(server_fd, reinterpret_cast<struct sockaddr *>(&address), addrlen) == -1)
	{
		std::cerr << strerror(errno);
		return(1);
	}

	if (listen(server_fd, 3) < 0) 
	{ 
		std::cerr << strerror(errno); 
		exit(EXIT_FAILURE); 
	}
	long	valread;
	int		client_fd;
	while(1)
	{
		printf("\n+++++++ Waiting for new connection ++++++++\n\n");
		client_fd = accept(server_fd, reinterpret_cast<struct sockaddr *>(&address), (socklen_t*)&addrlen);
		if (client_fd < 0)
		{
			std::cerr << strerror(errno);
			exit(EXIT_FAILURE);
		}
		char buffer[30000] = {0};
		valread = read( client_fd , buffer, 30000);
		std::cout << "The message was: " << buffer;
		std::string hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 13\n\nHello, World!";
		write(client_fd , hello.c_str() , hello.length());
		std::string response = "Good talking to you\n";
		send(client_fd, response.c_str(), response.size(), 0);
		printf("------------------Hello message sent-------------------\n");
		close(client_fd);
	}
	close(server_fd);
	return 0;
}