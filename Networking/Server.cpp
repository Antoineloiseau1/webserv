#include "Server.hpp"
#include "../parsing/parsing.hpp"
#include "../HTTP/Response.hpp"
#include <exception>

#define NUSERS 10

int	Server::getRequestFd() const { return _requestFd; }

char	**Server::getEnvp() const { return _envp; }

/* A ce stade, pas de monitoring des connections acceptees. pas de pb apparent.*/
void Server::_watchLoop() {
	int nev;
	_socklen = sizeof(_addr);

	while(1) {

		nev = kevent(_kq, NULL, 0, _evList, 32,  NULL);
		if (nev < 1) {
			perror("kevent() failed");
			exit(EXIT_FAILURE);
		}
		for (int i = 0; i < nev; i++) {
			if (_evList[i].flags & EV_EOF)
				{
					std::cout << "EOF: removing client connection from monitoring : " << _evList[i].ident << std::endl;
					EV_SET(&_evSet, _evList[i].ident, EVFILT_READ, EV_DELETE, 0, 0, NULL);
					if (kevent(_kq, &_evSet, 1, NULL, 0, NULL) < 0) {
						fprintf(stderr, "Problem adding kevent listener for client LOOP: %s\n",
						strerror(errno));
					}
					close(_evList[i].ident);
				}
			else if ((int)_evList[i].ident == _socket[0]->getFd()) {
				_accepter(_evList[i].ident);
			}
			else {
				if (_evList[i].flags & EVFILT_READ) {
					_handler(_evList[i].ident);
				}
				
			}
		}
	}
}

Server::Server(int domain, int service, int protocole, int *ports, int nbSocket, char **envp) : _envp(envp) {
	for (int i = 0; i < nbSocket; i++)
		this->_socket.push_back(new ListeningSocket(domain, service, protocole, ports[i]));
}

void	Server::start(void) {

	_kq = kqueue();
	EV_SET(&_evSet, _socket[0]->getFd(), EVFILT_READ, EV_ADD, 0, 0, NULL);
	if (kevent(_kq, &_evSet, 1, NULL, 0, NULL) < 0) {
		exit(1);
  	}
	_watchLoop();
}

// Handle incoming CONNECTION and add the connection socket to the kqueue
void	Server::_accepter(int server_fd) {
	struct sockaddr_in	address;
	socklen_t			addrlen = sizeof(address);

	_requestFd = accept(server_fd, reinterpret_cast<struct sockaddr *>(&address), &addrlen);
	if (this->_requestFd == -1) {
		std::cerr << "accept: " << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
	fcntl(_requestFd, F_SETFL, O_NONBLOCK);
	int opt = 1;
	if (setsockopt(_requestFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		perror("setsockopt : ");
		exit(EXIT_FAILURE);
	}
	EV_SET(&_evSet, _requestFd, EVFILT_READ, EV_ADD, 0, 0, NULL);
	if (kevent(_kq, &_evSet, 1, NULL, 0, NULL) < 0) {
        fprintf(stderr, "Problem adding kevent for client ACCEPTER: %s\n",
        strerror(errno));
    }
}

// Handle incoming data on accepted connections
void	Server::_handler(int client_fd) {
	memset(this->_requestBuffer, 0, sizeof(this->_requestBuffer));
	ssize_t n = recv(client_fd, _requestBuffer, sizeof(_requestBuffer), 0);
	if (n < 0) {
		perror("read() failed");
		EV_SET(&_evSet, client_fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
		if (kevent(_kq, &_evSet, 1, NULL, 0, NULL) < 0) {
			fprintf(stderr, "Problem adding kevent listener for client HANDLER: %s\n",
			strerror(errno));
		}
		close(client_fd);
	}
	else if (n == 0) {
		EV_SET(&_evSet, client_fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
		if (kevent(_kq, &_evSet, 1, NULL, 0, NULL) < 0) {
			fprintf(stderr, "Problem adding kevent listener for client HANDLER 2: %s\n",
			strerror(errno));
		}
	}
	else {
		// main case: handle received data (print for now)
		_requestBuffer[n] = '\0';
		std::cout << "Request From Client:\n" << _requestBuffer << std::endl;
		_responder(client_fd);
	}
}

void	Server::_responder(int client_fd) {
	Request		request(_requestBuffer);
	Response	response(request, *this);
	std::string res = response.buildResponse();
	std::cout << "Response from the server:\n" << res << std::endl;
	send(client_fd, res.c_str(), res.length(), 0);
	EV_SET(&_evSet, client_fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	if (kevent(_kq, &_evSet, 1, NULL, 0, NULL) < 0) {
		fprintf(stderr, "Problem adding kevent listener for client RESPON: %s\n",
		strerror(errno));
	}
	close(client_fd);
}

ListeningSocket	*Server::getSocket(void) const { return this->_socket[0]; }

Server::~Server(void) { delete this->_socket[0]; }
