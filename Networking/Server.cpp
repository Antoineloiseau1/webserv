#include "Server.hpp"
#include "../parsing/parsing.hpp"
#include "../HTTP/Response.hpp"
#include <exception>

#define NUSERS 10

static int test;
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
		std::cout << "++++ Number of events = "<< nev << std::endl;
		for (int i = 0; i < nev; i++) {
			if (_evList[i].flags & EV_EOF)
				{
					std::cout << "EOF: removing client connection from monitoring : " << _evList[i].ident << std::endl;
					EV_SET(&_evSet, _evList[i].ident, EVFILT_READ, EV_DELETE, 0, 0, NULL);
					if (kevent(_kq, &_evSet, 1, NULL, 0, NULL) < 0) {
						fprintf(stderr, "Problem adding kevent listener for client: %s\n",
						strerror(errno));
					}
					std::cout << "------- Closing fd pop: "<< _evList[i].ident << '\n';
					close(_evList[i].ident);
				}
			else if ((int)_evList[i].ident == _socket[0]->getFd()) {
				std::cout << "----*--- in accepter if: "<< _evList[i].ident << '\n';
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
		std::cout << "EROR\n";
  	}
	_watchLoop();
}

// Handle incoming CONNECTION and add the connection socket to the kqueue
void	Server::_accepter(int server_fd) {
	struct sockaddr_in	address;
	socklen_t			addrlen = sizeof(address);

	_requestFd = accept(server_fd, reinterpret_cast<struct sockaddr *>(&address), &addrlen);
	std::cout << ">>>>>>>>>> . TEST request fd : "<< _requestFd << "\n"; 
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
        fprintf(stderr, "Problem adding kevent for client: %s\n",
        strerror(errno));
    }
}

// Handle incoming data on accepted connections
void	Server::_handler(int client_fd) {
	std::cout << "IN HANDLER, FD= "<< client_fd<< std::endl;
	memset(this->_requestBuffer, 0, sizeof(this->_requestBuffer));
	ssize_t n = recv(client_fd, _requestBuffer, sizeof(_requestBuffer), 0);
	if (n < 0) {
		perror("read() failed");
		std::cout << "Closing fd lala: "<< client_fd << '\n';
		EV_SET(&_evSet, client_fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
		if (kevent(_kq, &_evSet, 1, NULL, 0, NULL) < 0) {
			fprintf(stderr, "Problem adding kevent listener for client: %s\n",
			strerror(errno));
		}
		close(client_fd);
		// exit(EXIT_FAILURE);
	}
	else if (n == 0) {
		EV_SET(&_evSet, client_fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
		if (kevent(_kq, &_evSet, 1, NULL, 0, NULL) < 0) {
			fprintf(stderr, "Problem adding kevent listener for client: %s\n",
			strerror(errno));
		}
	}
	else {
		// main case: handle received data (print for now)
		_requestBuffer[n] = '\0';
		std::cout << "========= "<< test++ << std::endl;
		printf("request : %s\n", _requestBuffer);
		_responder(client_fd, requestParse(_requestBuffer, *this));
	}
}

void	Server::_responder(int client_fd, Response *response) {

	/////////responder data is parts of response
	std::map<std::string, std::string> responder = response->getMap();
	std::string res = responder["version"] + ' ' + responder["status"] + responder["type"] + responder["length"] + responder["connexion"] + responder["body"];//"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 13\r\nConnection: keep-alive\r\n\r\nHello, World!";
	send(client_fd, res.c_str(), res.length(), 0);
	EV_SET(&_evSet, client_fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	if (kevent(_kq, &_evSet, 1, NULL, 0, NULL) < 0) {
		fprintf(stderr, "Problem adding kevent listener for client: %s\n",
		strerror(errno));
	}
	close(client_fd);
}

ListeningSocket	*Server::getSocket(void) const { return this->_socket[0]; }

Server::~Server(void) { delete this->_socket[0]; }
