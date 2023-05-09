#include "Socket.hpp"


/* Socket Default Constructor */
Socket::Socket(int domain, int service, int protocole, int port) {
	/* Initializing struct sockaddr_in */
	this->_address.sin_family = domain;
	this->_address.sin_port = htons(port);
	this->_address.sin_addr.s_addr = htonl(INADDR_ANY);
	this->_addrlen = sizeof(this->_address);
	memset(this->_address.sin_zero, '\0', sizeof(this->_address.sin_zero));

	/* Opening a socket */
	this->_fd = socket(domain, service, protocole);
	if (this->_fd == -1) {
		std::cerr << "socket: " << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
	int opt = 1;
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		perror("setsockopt : ");
		exit(EXIT_FAILURE);
	}
}

/* Socket virtual destructor */
Socket::~Socket(void) {}
