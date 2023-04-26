#include "ListeningSocket.hpp"


/*ListeningSocket Default Constructor */
ListeningSocket::ListeningSocket(int domain, int service, int protocole, int port) : Socket(domain, service, protocole, port) {

	/* Setting Socket Options */
	struct linger sl;
	setsockopt(this->_fd, SOL_SOCKET, SO_REUSEPORT, &sl, sizeof(this->_fd));

	/* Binding address and starting to listen*/
	establishConnection();
	std::cout << "+++++++ ListeningSocket Constuctor: Server has been opened +++++++" << std::endl << std::endl;
}

/* Bind socket to address and start to listent */
void	ListeningSocket::establishConnection(void) {
	/* Identifying(Naming) Socket */
	if (bind(this->_fd, reinterpret_cast<struct sockaddr*>(&this->_address), sizeof(this->_address)) == -1) {
		std::cerr << "bind: " << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}

	/* Getting Ready For Requests */
	if (listen(this->_fd, SOMAXCONN) == -1) 
	{ 
		std::cerr << "listen: " << strerror(errno) << std::endl; 
		exit(EXIT_FAILURE); 
	}
}

int	ListeningSocket::getFd(void) const { return this->_fd; }
socklen_t	ListeningSocket::getSockLen(void) const { return this->_addrlen; }
struct sockaddr_in	ListeningSocket::getAddress(void) const { return this->_address; }

ListeningSocket::~ListeningSocket(void) { close(this->_fd); }

