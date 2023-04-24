#include "Socket.hpp"

Socket::Socket(int domain, int service, int protocole, int port) {
	this->_address.sin_family = domain;
	this->_address.sin_port = htons(port);
	this->_address.sin_addr.s_addr = htonl(INADDR_ANY);
	this->_addrlen = sizeof(this->_address);
	memset(this->_address.sin_zero, '\0', sizeof(this->_address.sin_zero));

	this->_fd = socket(domain, service, protocole);
	if (this->_fd == -1) {
		std::cerr << "socket: " << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
}

Socket::~Socket(void) {}