#include "ListeningSocket.hpp"


/*ListeningSocket Default Constructor */
ListeningSocket::ListeningSocket(int domain, int service, int protocole, int port) : Socket(domain, service, protocole, port) {
	/* Binding address and starting to listen*/
	establishConnection();
	std::cout << "+++++++ Server is now listening on port \033[30;43m" << port <<  "\033[0m +++++++" << std::endl;
}

/* Bind socket to address and start to listent */
void	ListeningSocket::establishConnection(void) {
	/* Identifying(Naming) Socket */
	if (bind(this->_fd, reinterpret_cast<struct sockaddr*>(&this->_address), sizeof(this->_address)) == -1) {
		return;
	}

	/* Getting Ready For Requests */
	if (listen(this->_fd, SOMAXCONN) == -1) 
	{
		std::cerr << "listen: " << strerror(errno) << std::endl; 
		exit(EXIT_FAILURE); 
	}
}

int	ListeningSocket::getFd(void) const { return Socket::_fd; }

socklen_t	ListeningSocket::getSockLen(void) const { return this->_addrlen; }

struct sockaddr_in	ListeningSocket::getAddress(void) const { return this->_address; }

ListeningSocket::~ListeningSocket(void) { close(this->_fd); }


void	ListeningSocket::setClient(Client *newClient) {
	clients.push_back(newClient);
}

int		ListeningSocket::getOpenFd() {
	int res = 0;
	
	for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); it++)
	{
		res++;
	}
	return res;
}

Client	*ListeningSocket::getClient(int fd) {
	for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); it++) {
		if ((*it)->getFd() == fd)
		{
			return *it;
		}
	}
	return  NULL;
}

void	ListeningSocket::deleteClient(int fd) {
	for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); it++) {
		if ((*it)->getFd() == fd) {
			delete *it;
			clients.erase(it);
			break ;
		}
	}
}
