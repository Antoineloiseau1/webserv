#include "Server.hpp"
#include "../parsing/parsing.hpp"
#include "../HTTP/Response.hpp"

#define NUSERS 10
/* A ce stade, pas de monitoring des connections acceptees. pas de pb apparent.*/
void Server::_watchLoop() {
	int nev;
	_socklen = sizeof(_addr);

	while(1) {
		nev = kevent(_kq, &_evSet, 1, _evList, 32,  NULL);
		if (nev == -1) {
			perror("kevent() failed");
			exit(EXIT_FAILURE);
		}
		for (int i = 0; i < nev; ++i) {
			if (_evList[i].flags & EV_EOF) //si CTRL+C 
			{
				std::cout << "EOF: removing client connection from monitoring : " << _evList[i].ident << std::endl;
				EV_SET(&_evSet, _evList[i].ident, EVFILT_READ, EV_DELETE, 0, 0, NULL);
				close(_evList[i].ident);
			}
			else if ((int)_evList[i].ident == _socket[0]->getFd()) {
				close(_requestFd);
				_accepter(_evList[i].ident);
			}
			else {
				fd_set read_fds;
				FD_ZERO(&read_fds);
				FD_SET(_evList[i].ident, &read_fds);
				int num_ready = select(_evList[i].ident + 1, &read_fds, NULL, NULL, NULL);
				if (num_ready == -1) {
					std::cout << "ERROR : fd "<< _evList[i].ident << " not valid\n";
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
}

// Handle incoming data on accepted connections
void	Server::_handler(int client_fd) {
	memset(this->_requestBuffer, 0, sizeof(this->_requestBuffer));
	ssize_t n = recv(client_fd, _requestBuffer, sizeof(_requestBuffer), 0);
	if (n == -1) {
		perror("read() failed");
		exit(EXIT_FAILURE);
	}
	else if (n == 0) {
		EV_SET(&_evSet, client_fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	}
	else {
		// main case: handle received data (print for now)
		_requestBuffer[n] = '\0';

		/* PARSE AND CREATE A REQUEST (work in progress)*/

		EV_SET(&_evSet, client_fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
		printf("request : %s\n", _requestBuffer);
		_responder(client_fd, requestParse(_requestBuffer));
	}
}

void	Server::_responder(int client_fd, Response *response) {

	/////////responder data is parts of response
	std::map<std::string, std::string> responder = response->getMap();
	std::string res = responder["version"] + ' ' + responder["status"] + responder["type"] + responder["length"] + responder["connexion"] + responder["body"];//"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 13\r\nConnection: keep-alive\r\n\r\nHello, World!";
	send(client_fd, res.c_str(), res.length(), 0);
}

ListeningSocket	*Server::getSocket(void) const { return this->_socket[0]; }

Server::~Server(void) { delete this->_socket[0]; }
