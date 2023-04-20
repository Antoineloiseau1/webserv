#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#define PORT 8080

int main(void) {

	int server_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		std::cerr << strerror(errno);
		return (1);
	}
	struct sockaddr_in	address;
	address.sin_family = AF_INET;
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
	int		new_socket;
	while(1)
	{
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
		new_socket = accept(server_fd, reinterpret_cast<struct sockaddr *>(&address), (socklen_t*)&addrlen);
		if (new_socket < 0)
		{
			std::cerr << strerror(errno);
			exit(EXIT_FAILURE);
		}
        char buffer[30000] = {0};
        valread = read( new_socket , buffer, 30000);
		std::string hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 13\n\nHello, World!";
        write(new_socket , hello.c_str() , hello.length());
        printf("------------------Hello message sent-------------------\n");
        close(new_socket);
    }
    return 0;
}