#include "Server.hpp"
#include <fstream>
#include <fcntl.h>

Server::Server(int domain, int service, int protocole, int port, int backlog) {
	this->_socket = new ServerSocket(domain, service, protocole, port, backlog);
}

void	Server::start(void) {
    // open the HTML file
    std::ifstream file("./data/www/manon.html");
    if (!file.is_open())
    {
        std::cerr << "Error opening file" << std::endl;
        exit(EXIT_FAILURE);
    }

    // read the contents of the file into a string variable
    std::string content((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));

	while(true) {
		std::cout << "+++++++ Waiting for new connection ++++++++" << std::endl << std::endl;
		_accepter();
		_handler();
		_responder(content);
		close(this->_requestFd);
	}
}

void	Server::_accepter(void) {
	struct sockaddr_in	address;
	socklen_t			addrlen;
	int					r;

	this->_requestFd = accept(this->_socket->getFd(), reinterpret_cast<struct sockaddr *>(&address), &addrlen);
	if (this->_requestFd == -1) {
		delete this->_socket;
		std::cerr << "accept: " << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}

	fcntl(this->_requestFd, F_SETFL, O_NONBLOCK);
	memset(this->_requestBuffer, 0, sizeof(this->_requestBuffer));
	r = read(this->_requestFd, this->_requestBuffer, 30000);
	std::cout << "Server::Accepter: ";
	if (r > 0) {
		this->_requestBuffer[r] = 0;
		std::cout << " ########### Received " << r << " bytes ###########\n" << this->_requestBuffer << std::endl;
	}
	else
		std::cout << " bytes = " << r << " nothing to read" << std::endl;
}

void	Server::_handler(void) {
	std::cout << this->_requestBuffer << std::endl;
}

void	Server::_responder(std::string content) {
		// std::string hello = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: 354321354\n\n";
		// write(this->_requestFd , hello.c_str() , hello.length());
		// write(this->_requestFd, content.c_str(), content.length());
		// close(this->_requestFd);
		(void)content;
}

ServerSocket	*Server::getSocket(void) const { return this->_socket; }

Server::~Server(void) { delete this->_socket; }