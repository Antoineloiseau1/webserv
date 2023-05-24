#include "Server.hpp"
#include <exception>
#include <csignal>

#include "../parsing/parsing.hpp"
#include "../HTTP/Response.hpp"
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
	int	i = 0;

	fd_set	tmpRead;
	fd_set	tmpWrite;
	fd_set	tmpError;
	FD_ZERO(&tmpRead);
	FD_ZERO(&tmpWrite);
	FD_ZERO(&tmpError);
	signal(SIGINT, exit);

	while(_alive) 
	{
		if (i >= _data.getPortsNbr())
			i = 0;
		tmpRead = _readSet;
		tmpWrite = _writeSet;
		tmpError = _errorSet;
		std::cout << i << "whille\n";
		nbEvents = select(_getFdMax() + 1, &tmpRead, &tmpWrite, &tmpError, NULL);

///////////		/!\ work in progress
		if (FD_ISSET(_socket[i]->getFd(), &tmpError)) {
			std::cout << "ERROR SET SERVER\n";
			exit(1);
		}
		if (nbEvents < 1) {
			std::cerr << "Error: select(): " << strerror(errno) << std::endl;
			exit(EXIT_FAILURE);
		}
		if(FD_ISSET(_socket[i]->getFd(), &tmpRead) && _alive)
		{
			if (getOpenFd() > MAX_FD)
				_refuse(_socket[i]->getFd());
			else
				_accepter(_socket[i]->getFd(), _socket[i]);
		}

		size_t j = 0;
		for(std::vector<Client*>::iterator it = _socket[i]->clients.begin(); it != _socket[i]->clients.end() && nbEvents-- && _alive; it++)
		{
			if (j >= _socket[i]->clients.size()) //else it goes behind the size of client | maybe change it in the loop but ,for now, this works
			{
				std::cout << "ALED\n";
				break;
			}
			std::cout << "size : " << _socket[i]->clients.size() << std::endl;
			std::cout<< "heap buffer overflow " << j++ << std::endl;
			std::cout << i << "nbevnts "<< nbEvents << std::endl;
			Client *client = *it;
			if (FD_ISSET(client->getFd(), &tmpError)) {
				std::cout << "ERROR SET CLIENT\n";
				exit(1);
			}
			if(FD_ISSET(client->getFd(), &tmpRead)) {
				_handler(client, i);
			}
			if(FD_ISSET(client->getFd(), &tmpWrite)) {
				_responder(client, i);
			}
		}
		i++;
	}
	///////////		/!\ work in progress
	std::cout << "OUT OF WHILE LOOP \n";

}

Server::Server(int domain, int service, int protocole, int *ports, int nbSocket, char **envp, data& data) : _data(data), _envp(envp) {
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

// Handle incoming data on accepted connections
void	Server::_handler(Client *client, int i) {
	std::cout << "DEbug = dans handler, client fd : "<< client << std::endl;

	int	n = recv(client->getFd(), _requestBuffer, BUFFER_SIZE, 0);
	std::cout << "***** bytes read = " << n << std::endl;
	if (n < 0) {
		std::cerr << "error: recv: " << strerror(errno) << std::endl;
		FD_CLR(client->getFd(), &_readSet);
		close(client->getFd());
		_socket[i]->deleteClient(client->getFd()); //////////////i
	}
	else {
		_requestBuffer[n] = '\0';
//		std::cout << "requestBuffer = " << _requestBuffer << std::endl;
		/*******************POUR TOUT LE MONDE 1 X*****************************/
		if (strstr(_requestBuffer, "\r\n\r\n") != nullptr
			&& client->getStatus() == Client::INIT) {
//			std::cout << "Je suis dans le parsing header\n";
			client->createRequest(_requestBuffer);
			client->setStatus(Client::HEADER_PARSED);
		}
	
		if (client->getStatus() == Client::HEADER_PARSED 
			&& (client->getType() == Client::GET || client->getType() == Client::DELETE))
		{
//			std::cout << "*****Request From Client:\n" << _requestBuffer << std::endl;
			FD_CLR(client->getFd(), &_readSet);
			FD_SET(client->getFd(), &_writeSet);
			if (client->getFd() > _fdMax)
				_fdMax = client->getFd();
		}
		/* SOIT body buf est vide,
		SOIT body buf contient le prebody et un peu de data d'image
		soit body buf contient le user form */
		else if (client->getStatus() != Client::INIT && client->getType() == Client::POST_DATA)
		{	
			client->bytes += n;
			if (client->getStatus() < Client::READY_FOR_DATA) {
				if (client->getStatus() < Client::PARSING_PREBODY) {
//					std::cout << "++++parsing prebody 1 \n";
					client->setStatus(Client::PARSING_PREBODY);
					client->bytes = n - client->getRequest()->getHeaderLen();
					if (client->bytes > 0)
						client->parsePreBody(_requestBuffer + client->getRequest()->getHeaderLen() + 4, client->bytes);
				}
				else if (client->getStatus() == Client::PARSING_PREBODY){
//					std::cout << "++++parsing prebody 2 \n";
					client->parsePreBody(_requestBuffer, n);
					n = 0;
				}
			}
			if (client->bytes >= atoi(client->getRequest()->getHeaders()["Content-Length"].c_str())) {
//				std::cout << "++++BYTES = "<< client->bytes << " | atoi = " << atoi(client->getRequest()->getHeaders()["Content-Length"].c_str()) << std::endl;
				client->setStatus(Client::BODY_PARSED);
				FD_SET(client->getFd(), &_writeSet);
				FD_CLR(client->getFd(), &_readSet);
				if (client->getFd() > _fdMax)
					_fdMax = client->getFd();
				client->bytes = 0;
				client->getFile().close(); //closing file after finishing to write data
			}
			else if (client->getStatus() == Client::READY_FOR_DATA && n > 0) {
//				std::cout << "+++Writing in file... \n";
				client->writeInFile(_requestBuffer, n);
			}
		}
		else if (client->getStatus() != Client::INIT && client->getType() == Client::POST_FORM) {
			if (client->getStatus() < Client::PARSING_PREBODY) {
				client->bytes = n - client->getRequest()->getHeaderLen();
				client->setStatus(Client::PARSING_PREBODY);
				if (client->bytes > 0)
					client->setFormBody(_requestBuffer + client->getRequest()->getHeaderLen() + 4);
			}
			else if (client->getStatus() == Client::PARSING_PREBODY){
					client->setFormBody(_requestBuffer);
			}
			if (client->bytes >= atoi(client->getRequest()->getHeaders()["Content-Length"].c_str())) {
				std::cout << "++++BYTES = "<< client->bytes << " | atoi = " << atoi(client->getRequest()->getHeaders()["Content-Length"].c_str()) << std::endl;
				client->setStatus(Client::BODY_PARSED);
				FD_SET(client->getFd(), &_writeSet);
				FD_CLR(client->getFd(), &_readSet);
				if (client->getFd() > _fdMax)
					_fdMax = client->getFd();
				client->bytes = 0;
				client->getFile().close(); //closing file after finishing to write data
			}
//			std::cout << "PRE BODY FORM = " << client->getFormBody() << std::endl;
		}
	}
}

void	Server::_responder(Client *client, int i) {
	
	Response	response(*(client->getRequest()), *this);
	std::string res = response.buildResponse();

//	std::cout << "Response from the server:\n" << res << std::endl;
	send(client->getFd(), res.c_str(), res.length(), 0);
	disconnectClient(client, i);
}

void	Server::disconnectClient(Client *client, int i) {
	FD_CLR(client->getFd(), &_readSet);
	FD_CLR(client->getFd(), &_writeSet);
	FD_CLR(client->getFd(), &_errorSet);
	std::cout << "CLOSING CLIENT FD : " << client ->getFd() << std::endl;
	close(client->getFd());
	_socket[i]->deleteClient(client->getFd());
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

	for (int i = 0; i != _data.getPortsNbr(); i++)
		delete this->_socket[i]; }
