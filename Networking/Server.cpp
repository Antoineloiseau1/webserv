#include "Server.hpp"

#define NUSERS 10
/* A ce stade, pas de monitoring des connections acceptees. pas de pb apparent.*/
void Server::_watchLoop() {
	int nev;
	_socklen = sizeof(_addr);

	while(1) {
		nev = kevent(_kq, &_evSet, 1, _evList, 32,  NULL);
		// for (int i = 0; i < nev; i++)
		// {
		// 	std::cout << "TEST NEV pour i = " << i << " FD = " << _evList[i].ident << std::endl;
		// }
		if (nev == -1) {
			perror("kevent() failed");
			exit(EXIT_FAILURE);
		}
		for (int i = 0; i < nev; ++i) {
			std::cout << "--debut for loop : "<< i << std::endl;
			if (_evList[i].flags & EV_EOF) //si CTRL+C 
			{
				std::cout << "disconnect\n";
				int fd = _evList[i].ident;
				EV_SET(&_evSet, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
				close(fd);
			}
			else if ((int)_evList[i].ident == _socket[0]->getFd()) {
				_accepter(_evList[i].ident);
			}
			else {
				fd_set read_fds;
				FD_ZERO(&read_fds);
				FD_SET(_evList[i].ident, &read_fds);
				int num_ready = select(_evList[i].ident + 1, &read_fds, NULL, NULL, NULL);
				if (num_ready == -1) {
					std::cout << "ERROR \n";
					exit(1);
				}
				else if (num_ready == 0) {
					continue ;
				}
				else {
					// At least one socket is ready for reading
					if (FD_ISSET(_evList[i].ident, &read_fds)) {
						_handler(_evList[i].ident);
					}
				}
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
	_watchLoop();
}

// Handle incoming CONNECTION and add the connection socket to the kqueue
void	Server::_accepter(int server_fd) {
	struct sockaddr_in	address;
	socklen_t			addrlen;

	this->_requestFd = accept(server_fd, reinterpret_cast<struct sockaddr *>(&address), &addrlen);
	if (this->_requestFd == -1) {
		std::cerr << "accept: " << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
	fcntl(this->_requestFd, F_SETFL, O_NONBLOCK);
	EV_SET(&_evSet, this->_requestFd, EVFILT_READ, EV_ADD, 0, 0, NULL);
	std::cout << "ACCEPTED = " << _requestFd << std::endl;
}

// Handle incoming data on accepted connections
void	Server::_handler(int client_fd) {
	std::cout << "HANDLER : " << client_fd << std::endl;
	memset(this->_requestBuffer, 0, sizeof(this->_requestBuffer));
	ssize_t n = recv(client_fd, _requestBuffer, sizeof(_requestBuffer), 0);
	if (n == -1) {
		perror("read() failed");
		exit(EXIT_FAILURE);
	}
	else if (n == 0) {
		// Connection closed by client
		EV_SET(&_evSet, client_fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
		close(client_fd);
	}
	else {
		// main case: handle received data (print for now)
		_requestBuffer[n] = '\0';
		/* PARSE AND CREATE A REQUEST*/
		printf("Received data from %d: %s", client_fd, _requestBuffer);
		EV_SET(&_evSet, client_fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
		// close(client_fd); 
		// std::cout << "Closing 1 fd : " << client_fd << std::endl;
	}
}

// void	Server::_responder(int fd) {
// 		std::string hello = "HTTP/1.1 200 OK\n";
// 		write(fd , hello.c_str() , hello.length());
// }

ListeningSocket	*Server::getSocket(void) const { return this->_socket[0]; }

Server::~Server(void) { delete this->_socket[0]; }