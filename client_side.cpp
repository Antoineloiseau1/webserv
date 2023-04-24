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

void	client_side_send_request(int socketfd);

void	client_side_connect_to_server(std::string host, unsigned short int port, int& socketfd)
{

	  int status;

    struct addrinfo hints, *addr;
	  memset(&hints, 0, sizeof hints);
    //fine-tune hints according to which socket you want to open
    hints.ai_family = AF_INET; 
    hints.ai_socktype = SOCK_STREAM; 
    hints.ai_protocol = 0; //any protocol can be returned
    hints.ai_flags = AI_CANONNAME | AI_ALL | AI_ADDRCONFIG;

    const char* service = std::to_string(port).c_str();
	  const char* hostname = host.c_str();
    if ((status = getaddrinfo(hostname, service, &hints, &addr)) != 0) {
		std::cerr << "Error getting server address: " << gai_strerror(status) << std::endl;
		exit(EXIT_FAILURE);
	}
    socketfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
	if (connect(socketfd, addr->ai_addr, addr->ai_addrlen) == -1) 
	{ 
		std::cerr << "connect: " << strerror(errno) << std::endl; 
		exit(EXIT_FAILURE); 
	}
	client_side_send_request(socketfd);
}

void	client_side_send_request(int socketfd) {
	// Send the HTTP request
	const char* message = "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n";
  int bytes_sent = send(socketfd, message, strlen(message), 0);
  if (bytes_sent < 0) {
    std::cerr << "Error sending message" << std::endl;
    exit(EXIT_FAILURE);
  }

  // Receive the HTTP response
  char response[1024];
  int bytes_received = recv(socketfd, response, 1024, 0);
  if (bytes_received < 0) {
    std::cerr << "Error receiving message" << std::endl;
    exit(EXIT_FAILURE);
  }

  // Print the response
  std::cout << "Received " << bytes_received << " bytes: " << std::endl;
  std::cout.write(response, bytes_received);

  // Close the socket
  close(socketfd);
}