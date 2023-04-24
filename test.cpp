#include <arpa/inet.h>
#include <sys/uio.h>
#include <netdb.h>
#include <fcntl.h>
#include "./Networking/ServerSocket.hpp"
#include <fstream>

#define PORT 8080
#define FAMILY AF_INET
#define	SOCKTYPE SOCK_STREAM
// const int MAX_EVENTS = 100;
// const int TIMEOUT = -1;

void	manage_events() {
//use poll or kqueue
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


int main(void) {

	ServerSocket	server(FAMILY, SOCKTYPE, 0, PORT);

	long	bytes_read;
	int		request_fd;

    // open the HTML file
    std::ifstream file("./data/www/manon.html");
    if (!file.is_open())
    {
        std::cerr << "Error opening file" << std::endl;
        exit(EXIT_FAILURE);
    }

    // read the contents of the file into a string variable
    std::string content((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));

	while(1)
	{
		std::cout << "+++++++ Waiting for new connection ++++++++" << std::endl << std::endl;
		request_fd = server.acceptConnection(); 
		if (request_fd < 0)
		{
			std::cerr << "accept: " << strerror(errno) << std::endl;
			exit(EXIT_FAILURE);
		}

		/*  Setting fd to non blocking  */
		//fcntl(request_fd, F_SETFL, O_NONBLOCK);
		char buffer[30000] = {0};
		bytes_read = read( request_fd , buffer, 30000);
		std::cout <<  "(--------------- Received Request -----------------)\n" << buffer << std::endl;
		std::string hello = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: 354321354\n\n";
		write(request_fd , hello.c_str() , hello.length());
		write(request_fd, content.c_str(), content.length());
		close(request_fd);
	}
	return 0;
}