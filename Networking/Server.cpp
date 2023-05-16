#include "Server.hpp"
#include "../parsing/parsing.hpp"
#include "../HTTP/Response.hpp"
#include <exception>

#include "Client.hpp"
#define MAX_FD 230

int	Server::getOpenFd() {
	int res = _socket.size(); 
	
	for (std::vector<ListeningSocket*>::iterator it = _socket.begin(); it != _socket.end(); it++)
	{
		res += (*it)->getOpenFd();
	}
	return res;
}

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
		std::cout << "Test 1 : "<< _socket[0] << std::endl;
		fflush(stdout);
		for (int i = 0; i < nev; i++) {
		std::cout << "Test 2 : "<< _socket[0] << std::endl;
		fflush(stdout);
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
					std::cout << "Test 3 : "<< _socket[0] << std::endl;
		fflush(stdout);
				// _accepter(_evList[i].ident);
				if (getOpenFd() > MAX_FD)
					_refuse(_evList[i].ident);
				else
					_accepter(_evList[i].ident, _socket[0]);
			}
			else {
				if (_evList[i].flags & EVFILT_READ) {
					std::cout << "DEbug = event fd : "<< _evList[i].ident << std::endl;
					_handler((_socket[0])->getClient(_evList[i].ident));
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
void	Server::_accepter(int server_fd, ListeningSocket *sock) {
	struct sockaddr_in	address;
	socklen_t			addrlen = sizeof(address);
	std::cout << "Test 4 : "<< _socket[0] << std::endl;
		fflush(stdout);
	int fd = accept(server_fd, reinterpret_cast<struct sockaddr *>(&address), &addrlen);
	if (fd == -1) {
		std::cerr << "accept: " << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
	fcntl(fd, F_SETFL, O_NONBLOCK);
	int opt = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		perror("setsockopt : ");
		exit(EXIT_FAILURE);
	}
	EV_SET(&_evSet, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
	if (kevent(_kq, &_evSet, 1, NULL, 0, NULL) < 0) {
        fprintf(stderr, "Problem adding kevent for client ACCEPTER: %s\n",
        strerror(errno));
    }
	Client	*newClient = new Client(fd, sock->getFd());
	std::cout << "Test 4.1 : "<< _socket[0] << std::endl;
	fflush(stdout);
	sock->setClient(newClient);
	std::cout << "Test 4.2 : "<< _socket[0] << std::endl;
	fflush(stdout);
}

void	Server::_refuse(int server_fd) {
	struct sockaddr_in	address;
	socklen_t			addrlen;

	int fd = accept(server_fd, reinterpret_cast<struct sockaddr *>(&address), &addrlen);
	if (fd == -1) {
		std::cerr << "accept: " << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
	close(fd);
}

// Handle incoming data on accepted connections
void	Server::_handler(Client *client) {
		std::cout << "Test 5 : "<< _socket[0] << std::endl;
		fflush(stdout);
	std::cout << "DEbug = dans handler, client fd : "<< client->getFd() << std::endl;
	memset(this->_requestBuffer, 0, sizeof(this->_requestBuffer));
	ssize_t n = recv(client->getFd(), _requestBuffer, sizeof(_requestBuffer), 0);
	if (n < 0) {
		perror("read() failed");
		EV_SET(&_evSet, client->getFd(), EVFILT_READ, EV_DELETE, 0, 0, NULL);
		if (kevent(_kq, &_evSet, 1, NULL, 0, NULL) < 0) {
			fprintf(stderr, "Problem adding kevent listener for client HANDLER: %s\n",
			strerror(errno));
		}
		close(client->getFd());
		// _socket[0]->deleteClient(client->getFd());
	}
	else {
		// main case: handle received data (print for now)
		std::cout << "Test 5.1 : "<< _socket[0] << std::endl;
		fflush(stdout);
		_requestBuffer[n] = '\0';
		std::cout << "Test 5.2 : "<< _socket[0] << std::endl;
		fflush(stdout);
		// std::cout << "DEBUUUG : request buf = " << _requestBuffer << std::endl;
		int	bufBytes = client->getReqBuf().size();
		client->addOnReqBuf(_requestBuffer + bufBytes);
		std::cout << "DEBUUUG : request line = " << client->getReqBuf() << std::endl;
		// std::cout << "*****Request From Client:\n" << client->getReqBuf() << std::endl;
		_responder(client);
	}
}

void	Server::_responder(Client *client) {
	Request		request(client->getReqBuf());
	Response	response(request, *this);
	std::string res = response.buildResponse();

	std::cout << "Response from the server:\n" << res << std::endl;
	send(client->getFd(), res.c_str(), res.length(), 0);
	
	EV_SET(&_evSet, client->getFd(), EVFILT_READ, EV_DELETE, 0, 0, NULL);
	if (kevent(_kq, &_evSet, 1, NULL, 0, NULL) < 0) {
		fprintf(stderr, "Problem adding kevent listener for client RESPON: %s\n",
		strerror(errno));
	}
	close(client->getFd());
	// std::cout << "Client test fd : "<< client->getFd() << std::endl;
	// std::cout << "Socket [0]: "<< _socket[0] << std::endl;
	// _socket[0]->deleteClient(client->getFd());
	
}

ListeningSocket	*Server::getSocket(void) const { return this->_socket[0]; }

ListeningSocket	*Server::getSocket(int fd) {
	for (std::vector<ListeningSocket*>::iterator it = _socket.begin(); it != _socket.end(); it++)
	{
		std::cout << "TEST 1  DANS GET SOCKET = "<< (*it) << std::endl;
		std::cout << "TEST DANS GET SOCKET = "<< (*it)->getFd() << std::endl;
		fflush(stdout);
		if ((*it)->getFd() == fd)
			return *it;
	}
	return nullptr;
}

Server::~Server(void) {
	std::cout << "\n\n ************DELETION DELETION DELETION********* \n";
	delete this->_socket[0]; }
