#include "ServerSocket.hpp"

#define BACKLOG 1

ServerSocket::ServerSocket(int domain, int service, int protocole, int port) : Socket(domain, service, protocole, port) {
	/* Setting Socket Options */
	struct linger sl;
	setsockopt(this->_fd, SOL_SOCKET, SO_REUSEPORT, &sl, sizeof(this->_fd));

	/* Binding address */
	this->establishConnection();

	/* Getting Ready For Requests */
	if (listen(this->_fd, BACKLOG) == -1) 
	{ 
		std::cerr << "listen: " << strerror(errno) << std::endl; 
		exit(EXIT_FAILURE); 
	}
	std::cout << "+++++++ webserv has been opened +++++++" << std::endl << std::endl;
}

ServerSocket::~ServerSocket(void) { close(this->_fd); }

void	ServerSocket::establishConnection(void) {
	if (bind(this->_fd, reinterpret_cast<struct sockaddr*>(&this->_address), sizeof(this->_address)) == -1) {
		std::cerr << "bind: " << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
}

int	ServerSocket::getFd(void) const { return this->_fd; }
struct sockaddr_in	ServerSocket::getAddress(void) const { return this->_address; }

int	ServerSocket::acceptConnection(void) {
		int	fd;
		fd = accept(this->_fd, reinterpret_cast<struct sockaddr*>(&this->_address), &this->_addrlen);
		if (this->_fd < 0)
		{
			std::cerr << "accept: " << strerror(errno) << std::endl;
			exit(EXIT_FAILURE);
		}
		return (fd);
}

