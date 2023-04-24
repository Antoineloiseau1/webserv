#include "ServerSocket.hpp"

ServerSocket::ServerSocket(int domain, int service, int protocole, int port, int backlog) : Socket(domain, service, protocole, port) {
	/* Setting Socket Options */
	struct linger sl;
	setsockopt(this->_fd, SOL_SOCKET, SO_REUSEPORT, &sl, sizeof(this->_fd));

	/* Binding address */
	establishConnection();

	/* Getting Ready For Requests */
	if (listen(this->_fd, backlog) == -1) 
	{ 
		std::cerr << "listen: " << strerror(errno) << std::endl; 
		exit(EXIT_FAILURE); 
	}
	std::cout << "+++++++ ServerSocket has been opened +++++++" << std::endl << std::endl;
}

ServerSocket::~ServerSocket(void) { close(this->_fd); }

void	ServerSocket::establishConnection(void) {
	if (bind(this->_fd, reinterpret_cast<struct sockaddr*>(&this->_address), sizeof(this->_address)) == -1) {
		std::cerr << "bind: " << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
}

int	ServerSocket::getFd(void) const { return this->_fd; }
socklen_t	ServerSocket::getSockLen(void) const { return this->_addrlen; }
struct sockaddr_in	ServerSocket::getAddress(void) const { return this->_address; }

