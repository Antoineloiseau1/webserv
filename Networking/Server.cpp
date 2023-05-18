#include "Server.hpp"
#include "../parsing/parsing.hpp"
#include "../HTTP/Response.hpp"
#include <exception>

#include "Client.hpp"
#define MAX_FD 230

int	Server::getRequestFd() const { return _requestFd; }

char	**Server::getEnvp() const { return _envp; }

/* A ce stade, pas de monitoring des connections acceptees. pas de pb apparent.*/
int	Server::getOpenFd() {
	int res = _socket.size(); 
	
	for (std::vector<ListeningSocket*>::iterator it(_socket.begin()); it != _socket.end(); it++)
	{
		res += (*it)->getOpenFd();
	}
	return res;
}

int	Server::_getFdMax(void) {
	_fdMax = 0;
	for(std::vector<ListeningSocket*>::iterator s(_socket.begin()); s != _socket.end(); ++s)
	{
		ListeningSocket *socket = *s;
		if(socket->getFd() > _fdMax)
			_fdMax = socket->getFd();
		for(std::vector<Client*>::iterator c(socket->clients.begin()); c != socket->clients.end(); ++c)
		{
			Client *client = *c;
			if (client->getFd() > _fdMax)
				_fdMax = client->getFd();
		}
	}
	return(_fdMax);
}

/* A ce stade, pas de monitoring des connections acceptees. pas de pb apparent.*/
void Server::_watchLoop() {
	int 	nbEvents;
	fd_set	tmpRead;
	fd_set	tmpWrite;
	FD_ZERO(&tmpRead);
	FD_ZERO(&tmpWrite);

	while(true) 
	{
		tmpRead = _readSet;
		tmpWrite = _writeSet;
		nbEvents = select(_getFdMax() + 1, &tmpRead, &tmpWrite, NULL, NULL);
		if (nbEvents < 1) {
			std::cerr << "Error: select(): " << strerror(errno) << std::endl;
			exit(EXIT_FAILURE);
		}
		if(FD_ISSET(_socket[0]->getFd(), &tmpRead))
		{
			if (getOpenFd() > MAX_FD)
				_refuse(_socket[0]->getFd());
			else
				_accepter(_socket[0]->getFd(), _socket[0]);
		}
		for(std::vector<Client*>::iterator it(_socket[0]->clients.begin()); it != _socket[0]->clients.end(); ++it)
		{
			Client *client = *it;
			if(FD_ISSET(client->getFd(), &tmpRead)) {
				_handler(client);
				break;
			}
			if(FD_ISSET(client->getFd(), &tmpWrite)) {
				_responder(client);
				break;
			}
		}
	}
}

Server::Server(int domain, int service, int protocole, int *ports, int nbSocket, char **envp) : _envp(envp) {
	for (int i = 0; i < nbSocket; i++)
		this->_socket.push_back(new ListeningSocket(domain, service, protocole, ports[i]));
}

void	Server::start(void)
{
	_socklen = sizeof(_addr);
	FD_ZERO(&_readSet);
	FD_ZERO(&_writeSet);
	_fdMax = _socket.front()->getFd();
	for(std::vector<ListeningSocket*>::iterator it(_socket.begin()); it != _socket.end(); ++it)
	{
		ListeningSocket *socket = *it;
		FD_SET(socket->getFd(), &_readSet);
		if(socket->getFd() > _fdMax)
			_fdMax = socket->getFd();
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
	FD_SET(fd, &_readSet);
	if(fd > _fdMax)
		_fdMax = fd;
	Client* newClient = new Client(fd, sock->getFd());
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

	memset(this->_requestBuffer, 0, sizeof(this->_requestBuffer));

	ssize_t n = recv(client->getFd(), _requestBuffer, sizeof(_requestBuffer), 0);
	if (n < 0) {
		std::cerr << "error: recv: " << strerror(errno) << std::endl;
		FD_CLR(client->getFd(), &_readSet);
		close(client->getFd());
		std::cout << "\n\n\nCLOSED\n\n\n";
		_socket[0]->deleteClient(client->getFd());
	}
	else {
		// main case: handle received data (print for now)
		_requestBuffer[n] = '\0';
		std::cout << "requestBuffer = " << _requestBuffer << std::endl;
		// std::cout << "DEBUUUG : request buf = " << _requestBuffer << std::endl;
		int	bufBytes = client->getReqBuf().size();
		std::cout << "_socket: " << _socket[0] << std::endl;
		client->addOnReqBuf(_requestBuffer + bufBytes);
		// std::cout << "DEBUUUG : request line = " << client->getReqBuf() << std::endl;
		if (client->getStatus() == Client::INIT) {
			client->createRequest(client->getReqBuf());
			client->setStatus(Client::HEADER_PARSED);
		}
		if (client->getRequest()->getType() == "GET") {
			std::cout << "*****Request From Client:\n" << client->getReqBuf() << std::endl;
			FD_SET(client->getFd(), &_writeSet);
			if (client->getFd() > _fdMax)
				_fdMax = client->getFd();
		}
		else if (client->getRequest()->getType() == "POST")
		{
			std::cout << "\n SIZE OF BODY RECEIVED : " << client->getReqBuf().size() << "vs : " << atoi(client->getRequest()->getHeaders()["Content-Length"].c_str()) << std::endl;
			if (client->getReqBuf().size() < static_cast<unsigned long>(atoi(client->getRequest()->getHeaders()["Content-Length"].c_str())))
				return;
			else {
				client->getRequest()->parsingBody();
				client->setStatus(Client::BODY_PARSED);
				FD_SET(client->getFd(), &_writeSet);
				if (client->getFd() > _fdMax)
					_fdMax = client->getFd();
			}
		}
	}
}

void	Server::_responder(Client *client) {
	
	Response	response(*(client->getRequest()), *this);
	std::string res = response.buildResponse();

	std::cout << "Response from the server:\n" << res << std::endl;
	send(client->getFd(), res.c_str(), res.length(), 0);
	FD_CLR(client->getFd(), &_readSet);
	FD_CLR(client->getFd(), &_writeSet);
	std::cout << "CLOSING CLIENT FD : " << client ->getFd() << std::endl;
	close(client->getFd());
	_socket[0]->deleteClient(client->getFd());
}

ListeningSocket	*Server::getSocket(void) const { return this->_socket[0]; }

ListeningSocket	*Server::getSocket(int fd) {
	for (std::vector<ListeningSocket*>::iterator it = _socket.begin(); it != _socket.end(); it++)
	{
		if ((*it)->getFd() == fd)
			return *it;
	}
	return nullptr;
}

Server::~Server(void) {
	std::cout << "\n\n ************DELETION DELETION DELETION********* \n";
	delete this->_socket[0]; }
