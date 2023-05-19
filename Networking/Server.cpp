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
				// _accepter(_evList[i].ident);
				if (getOpenFd() > MAX_FD)
					_refuse(_evList[i].ident);
				else
					_accepter(_evList[i].ident, _socket[0]);
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
void	Server::_accepter(int server_fd, ListeningSocket *sock) {
	struct sockaddr_in	address;
	socklen_t			addrlen = sizeof(address);

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
	Client	*newClient = new Client(fd);
	sock->setClient(newClient);
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
	std::cout << "DEbug = dans handler, client fd : "<< client << std::endl;

	static long	bytes = 0;

	int	n = recv(client->getFd(), _requestBuffer, BUFFER_SIZE, 0);
	std::cout << "***** n = " << n << std::endl;
	if (n < 0) {
		perror("read() failed");
		EV_SET(&_evSet, client_fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
		if (kevent(_kq, &_evSet, 1, NULL, 0, NULL) < 0) {
			fprintf(stderr, "Problem adding kevent listener for client HANDLER: %s\n",
			strerror(errno));
		}
		close(client_fd);
	}
	// else if (n == 0) {
	// 	EV_SET(&_evSet, client_fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	// 	if (kevent(_kq, &_evSet, 1, NULL, 0, NULL) < 0) {
	// 		fprintf(stderr, "Problem adding kevent listener for client HANDLER 2: %s\n",
	// 		strerror(errno));
	// 	}
	// }
	else {
		// main case: handle received data (print for now)
		_requestBuffer[n] = '\0';
		std::cout << "requestBuffer = " << _requestBuffer << std::endl;
/* *****A voir si on arrive toujours a choper tous les headers en 1 passage,
sinon il faudra sauvegarder le request buffer et concatener**************/
		if (strstr(_requestBuffer, "\r\n\r\n") != nullptr
			&& client->getStatus() == Client::INIT)
		{
			std::cout << "Je suis dans le parsing header\n";
			client->createRequest(_requestBuffer);
			client->setStatus(Client::HEADER_PARSED);
			client->setBodyBufSize(n - client->getRequest()->getHeaderLen());
			bytes = client->getBodyBufSize();
			client->setBodyBuf(_requestBuffer + client->getRequest()->getHeaderLen() + 4);
		}
		if (client->getStatus() == Client::HEADER_PARSED && client->getRequest()->getType() == "GET") {
			std::cout << "*****Request From Client:\n" << _requestBuffer << std::endl;
			FD_CLR(client->getFd(), &_readSet);
			FD_SET(client->getFd(), &_writeSet);
			if (client->getFd() > _fdMax)
				_fdMax = client->getFd();
		}
		else if (client->getStatus() == Client::HEADER_PARSED && client->getRequest()->getType() == "POST")
		{ 
			bytes += n;
			if (client->getRequest()->getHeaders()["Content-Type"].find("multipart/form-data") != std::string::npos) {
				/* ******trying to upload a file*****************/
				if (strstr(client->getBodyBuf(), "Content-Type") != nullptr 
					&& client->getStatus() == Client::HEADER_PARSED)  {
					std::cout << "+++ENTERING SETPREBODY \n";
					client->setPreBody();
					client->setBodyBuf(client->getBodyBuf() + client->getPreBodySize());
					client->setStatus(Client::PRE_BODY_PARSED);
					client->writeInFile(client->getBodyBuf(), client->getBodyBufSize());
					client->readyForData = true;
					//parser le prebody dans la request
				}
				if (bytes >= atoi(client->getRequest()->getHeaders()["Content-Length"].c_str())) {
					std::cout << "++++BYTES = "<< bytes << " | atoi = " << atoi(client->getRequest()->getHeaders()["Content-Length"].c_str()) << std::endl;
					client->setStatus(Client::BODY_PARSED);
					FD_SET(client->getFd(), &_writeSet);
					FD_CLR(client->getFd(), &_readSet);
					bytes = 0;
					//fermer le file;
					if (client->getFd() > _fdMax)
						_fdMax = client->getFd();
				}
				else if (client->readyForData) {
					std::cout << "+++ENTERING GetStatus \n";
					client->writeInFile(_requestBuffer, n);
				}
			}
		}
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
