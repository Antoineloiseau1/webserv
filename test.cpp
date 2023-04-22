#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <cerrno> //linux
#include <string.h> //linux
#include <stdlib.h> // linux
 #include <sys/types.h>
#include <event2/event.h>
#include <sys/time.h>
#include <vector>

#define PORT 8081
#define FAMILY AF_INET
#define	SOCKTYPE SOCK_STREAM
const int MAX_EVENTS = 100;
const int TIMEOUT = -1;

void	manage_events() {
//use poll or kqueue
}

int main(void) {

	/* Opening a socket */
	int server_fd = socket(PF_INET, SOCKTYPE, 0);
	if (server_fd == -1)
	{
		std::cerr << "socket: " << strerror(errno) << std::endl;
		return (EXIT_FAILURE);
	}

	/* Setting socket's parameters */
	struct linger sl;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &sl, sizeof(server_fd));

	/* Naming the socket (bind): linking it to a port */
	struct sockaddr_in	address;
	socklen_t	addrlen = sizeof(address);
	address.sin_family = FAMILY;
	address.sin_port = htons(PORT);
	address.sin_addr.s_addr = htonl(INADDR_ANY);
 	memset(address.sin_zero, '\0', sizeof address.sin_zero);
	if (bind(server_fd, reinterpret_cast<struct sockaddr *>(&address), addrlen) == -1)
	{
		std::cerr << "bind: " << strerror(errno) << std::endl;
		return(EXIT_FAILURE);
	}

	/* Accept connexions to server */
	if (listen(server_fd, 10) == -1) 
	{ 
		std::cerr << "listen: " << strerror(errno) << std::endl; 
		exit(EXIT_FAILURE); 
	}
	std::cout << "+++++++ webserv has been opened +++++++" << std::endl;


	long	bytes_read;
	int		request_fd;

	manage_events();
	while(1)
	{
		std::cout << "+++++++ Waiting for new connection ++++++++" << std::endl;
		request_fd = accept(server_fd, reinterpret_cast<struct sockaddr*>(&address), &addrlen);
		if (request_fd < 0)
		{
			std::cerr << "accept: " << strerror(errno) << std::endl;
			exit(EXIT_FAILURE);
		}
		/* ???? Setting fd to non blocking ???? */
		fcntl(request_fd, F_SETFL, O_NONBLOCK);
		char buffer[30000] = {0};
		bytes_read = read( request_fd , buffer, 30000);
		std::cout << "The message was: " << buffer << " with length " << bytes_read;
		std::string hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 13\n\nHello, World!";
		write(request_fd , hello.c_str() , hello.length());
		std::string response = "Good talking to you\n";
		send(request_fd, response.c_str(), response.size(), 0);
		std::cout << ("------------------Hello message sent-------------------") << std::endl;
		close(request_fd);
	}
	close(server_fd);
	return 0;
}