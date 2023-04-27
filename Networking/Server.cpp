#include "Server.hpp"

#define NUSERS 10

void Server::_watchLoop() {
	int nev;
	_socklen = sizeof(_addr);

	while(1) {
		nev = kevent(_kq, NULL, 0, _evList, 32, NULL);
		if (nev == -1) {
			perror("kevent() failed");
			exit(EXIT_FAILURE);
		}
		for (int i = 0; i < nev; ++i) {
			if (_evList[i].flags & EV_EOF) {
				std::cout << "disconnect\n";
				int fd = _evList[i].ident;
				EV_SET(&_evSet, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
				if (kevent(_kq, &_evSet, 1, NULL, 0, NULL) == -1)
					std::cerr << "kevent: " << strerror(errno) << std::endl;
				close(fd);
			}
			else if ((int)_evList[i].ident == _socket[0]->getFd()) {
				// Handle incoming CONNECTION and add the connection socket to the kqueue for monitoring
				_accepter(_evList[i].ident);
			}
			else {
				// Handle incoming data
				_handler(_evList[i].ident);
			}
		}
	}
}


Server::Server(int domain, int service, int protocole, int *ports, int nbSocket) {
	for (int i = 0; i < nbSocket; i++)
		this->_socket.push_back(new ListeningSocket(domain, service, protocole, ports[i]));
}

void	Server::start(void) {
	// open the HTML file
	// std::ifstream file("./data/www/manon.html");
	// if (!file.is_open())
	// {
	//     std::cerr << "Error opening file" << std::endl;
	//     exit(EXIT_FAILURE);
	// }

	// // read the contents of the file into a string variable
	// std::string content((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
	
	_kq = kqueue();
	EV_SET(&_evSet, _socket[0]->getFd(), EVFILT_READ, EV_ADD, 0, 0, NULL);
	if (kevent(_kq, &_evSet, 1, NULL, 0, NULL) == -1)
		std::cerr << "kevent: " << strerror(errno) << std::endl;

	_watchLoop();
}

void	Server::_accepter(int server_fd) {
	struct sockaddr_in	address;
	socklen_t			addrlen;
	// int					r;

	this->_requestFd = accept(server_fd, reinterpret_cast<struct sockaddr *>(&address), &addrlen);
	if (this->_requestFd == -1) {
		std::cerr << "accept: " << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
	fcntl(this->_requestFd, F_SETFL, O_NONBLOCK);
	// memset(this->_requestBuffer, 0, sizeof(this->_requestBuffer));
	// r = read(this->_requestFd, this->_requestBuffer, 30000);
	// std::cout << "Server::Accepter: ";
	// if (r > 0) {
	// 	this->_requestBuffer[r] = 0;
	// 	std::cout << " ########### Received " << r << " bytes ###########\n" << std::endl;
	// }
	// else
	// 	std::cout << " bytes = " << r << " nothing to read" << std::endl;
	EV_SET(&_evSet, this->_requestFd, EVFILT_READ, EV_ADD, 0, 0, NULL);
	if (kevent(_kq, &_evSet, 1, NULL, 0, NULL) == -1) {
		perror("kevent() failed");
		exit(EXIT_FAILURE);
	}
}

void	Server::_handler(int client_fd) {
	char buf[1024];
	
	ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
	if (n == -1) {
		perror("read() failed");
		exit(EXIT_FAILURE);
	}
	else if (n == 0) {
		// Connection closed by client
		close(client_fd);
		EV_SET(&_evSet, client_fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
		if (kevent(_kq, &_evSet, 1, NULL, 0, NULL) == -1) {
			perror("kevent() failed");
			exit(EXIT_FAILURE);
		}
	}
	else {
		// Handle received data
		buf[n] = '\0';
		printf("Received data from %d: %s", client_fd, buf);
		// _responder(client_fd);
		close(client_fd);
	}
	// std::cout << this->_requestBuffer << std::endl;
}

// void	Server::_responder(int fd) {
// 		std::string hello = "HTTP/1.1 200 OK\n";
// 		write(fd , hello.c_str() , hello.length());
// }

ListeningSocket	*Server::getSocket(void) const { return this->_socket[0]; }

Server::~Server(void) { delete this->_socket[0]; }