#include "Server.hpp"
#include <fstream>

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
		std::cout <<  "(--------------- Received Request -----------------)\n" << std::endl;
		_handler();
		_responder(content);
	}
}

void	Server::_accepter(void) {
	struct sockaddr_in	address = this->_socket->getAddress();
	socklen_t			addrlen = sizeof(address);
	this->_requestFd = accept(this->_socket->getFd(), reinterpret_cast<struct sockaddr *>(&address), &addrlen);
	if (this->_requestFd == -1) {
		delete this->_socket;
		std::cerr << "accept: " << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}

	//fcntl(request_fd, F_SETFL, O_NONBLOCK);
	read(this->_requestFd, _requestBuffer, 30000);
}

void	Server::_handler(void) {
	std::cout << _requestBuffer << std::endl;
}

void	Server::_responder(std::string content) {
		std::string hello = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: 354321354\n\n";
		write(this->_requestFd , hello.c_str() , hello.length());
		write(this->_requestFd, content.c_str(), content.length());
		close(this->_requestFd);
}

ServerSocket	*Server::getSocket(void) const { return this->_socket; }

Server::~Server(void) { delete this->_socket; }