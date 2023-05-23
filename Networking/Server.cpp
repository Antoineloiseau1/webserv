#include "Server.hpp"
#include "../parsing/parsing.hpp"
#include "../HTTP/Response.hpp"
#include <exception>
#include <csignal>

#include "Client.hpp"
#define MAX_FD 230

bool	_alive = true;

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

void	Server::exit(int sig)
{
	(void)sig;

	std::cout << "\n" << "exiting...\n";
	_alive = false;
}

/* A ce stade, pas de monitoring des connections acceptees. pas de pb apparent.*/
void Server::_watchLoop() {
	int 	nbEvents;
	fd_set	tmpRead;
	fd_set	tmpWrite;
	fd_set	tmpError;
	FD_ZERO(&tmpRead);
	FD_ZERO(&tmpWrite);
	FD_ZERO(&tmpError);
	signal(SIGINT, exit);

	while(_alive) 
	{
		tmpRead = _readSet;
		tmpWrite = _writeSet;
		tmpError = _errorSet;
		std::cout << "while\n";
		nbEvents = select(_getFdMax() + 1, &tmpRead, &tmpWrite, &tmpError, NULL);
		if (FD_ISSET(_socket[0]->getFd(), &tmpError)) {
			std::cout << "ERROR SET SERVER\n";
			exit(1);
		}
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
		for(std::vector<Client*>::iterator it = _socket[0]->clients.begin(); it != _socket[0]->clients.end() && nbEvents--; it++)
		{
			std::cout << nbEvents << std::endl;
			Client *client = *it;
			if (FD_ISSET(client->getFd(), &tmpError)) {
				std::cout << "ERROR SET CLIENT\n";
				exit(1);
			}
			if(FD_ISSET(client->getFd(), &tmpRead)) {
				_handler(client);
			}
			if(FD_ISSET(client->getFd(), &tmpWrite)) {
				_responder(client);
			}
		}
	}
	std::cout << "OUT OF WHILE LOOP \n";
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
	FD_ZERO(&_errorSet);
	_fdMax = _socket.front()->getFd();
	for(std::vector<ListeningSocket*>::iterator it(_socket.begin()); it != _socket.end(); ++it)
	{
		ListeningSocket *socket = *it;
		FD_SET(socket->getFd(), &_readSet);
		FD_SET(socket->getFd(), &_errorSet);
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
	FD_SET(fd, &_errorSet);
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

void	Server::setToWrite(Client *client) {
	FD_CLR(client->getFd(), &_readSet);
	FD_SET(client->getFd(), &_writeSet);
	if (client->getFd() > _fdMax)
		_fdMax = client->getFd();
}

// Handle incoming data on accepted connections
void	Server::_handler(Client *client) {

	int	n = recv(client->getFd(), _requestBuffer, BUFFER_SIZE, 0);
	// std::cout << "***** n = " << n << std::endl;
	if (n < 0) { // mettre <=0 et gerer proprement pour la boucle infinie
		std::cerr << "error: recv: " << strerror(errno) << std::endl;
		FD_CLR(client->getFd(), &_readSet);
		close(client->getFd());
		_socket[0]->deleteClient(client->getFd());
	}
	else {
		_requestBuffer[n] = '\0';
		// std::cout << "requestBuffer = " << _requestBuffer << std::endl;
		/*******************POUR TOUT LE MONDE 1 X*****************************/
		if (strstr(_requestBuffer, "\r\n\r\n") != nullptr
			&& client->getStatus() == Client::INIT) {
			client->createRequest(_requestBuffer);
			client->setStatus(Client::HEADER_PARSED);
		}
		if (client->getStatus() > Client::INIT) {
			switch (client->getType()) {
				case Client::GET_DELETE:
					setToWrite(client);
					break;
				case Client::POST_DATA:
					client->bytes += n;
					if (client->getStatus() < Client::READY_FOR_DATA) {
						if (client->getStatus() < Client::PARSING_PREBODY) {
							client->setStatus(Client::PARSING_PREBODY);
							client->bytes = n - client->getRequest()->getHeaderLen();
							if (client->bytes > 0)
								client->parsePreBody(_requestBuffer + client->getRequest()->getHeaderLen() + 4, client->bytes);
						}
						else if (client->getStatus() == Client::PARSING_PREBODY){
							client->parsePreBody(_requestBuffer, n);
							n = 0;
						}
					}
					break;
				case Client::POST_FORM:
					if (client->getStatus() < Client::PARSING_PREBODY) {
						client->bytes = n - client->getRequest()->getHeaderLen();
						client->setStatus(Client::PARSING_PREBODY);
						if (client->bytes > 0)
							client->setFormBody(_requestBuffer + client->getRequest()->getHeaderLen() + 4);
					}
					else if (client->getStatus() == Client::PARSING_PREBODY){
							client->setFormBody(_requestBuffer);
					}
					break;
			}
		}
		if (client->bytes >= atoi(client->getRequest()->getHeaders()["Content-Length"].c_str())) {
				// std::cout << "++++BYTES = "<< client->bytes << " | atoi = " << atoi(client->getRequest()->getHeaders()["Content-Length"].c_str()) << std::endl;
				client->setStatus(Client::BODY_PARSED); //ne sert surement a rien
				setToWrite(client);
				client->getRequest()->setFormBody(client->getFormBody());
				client->bytes = 0;
				client->getFile().close(); //closing file after finishing to write data
		}
		else if (client->getStatus() == Client::READY_FOR_DATA && n > 0) {
				client->writeInFile(_requestBuffer, n);
			}
	}
}

void	Server::_responder(Client *client) {
	
	Response	response(*(client->getRequest()), *this);
	std::string res = response.buildResponse();

	std::cout << "Response from the server:\n" << res << std::endl;
	send(client->getFd(), res.c_str(), res.length(), 0);
	disconnectClient(client);
}

void	Server::disconnectClient(Client *client) {
	FD_CLR(client->getFd(), &_readSet);
	FD_CLR(client->getFd(), &_writeSet);
	FD_CLR(client->getFd(), &_errorSet);
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
