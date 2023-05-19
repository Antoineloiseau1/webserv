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
	FD_ZERO(&tmpRead);
	FD_ZERO(&tmpWrite);
	signal(SIGINT, exit);

	while(_alive) 
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



	ssize_t n = recv(client->getFd(), _requestBuffer, BUFFER_SIZE, 0);
	std::cout << "***** n = " << n << std::endl;
	if (n < 0) {
		std::cerr << "error: recv: " << strerror(errno) << std::endl;
		FD_CLR(client->getFd(), &_readSet);
		close(client->getFd());
		_socket[0]->deleteClient(client->getFd());
	}
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
		{/* -COMPTER LE NOMBRE DE BYTES RECEIVED TO KNOW IF WE HAVE EVERYTHING FROM THE BODY
			- SI LE BODY CONCERNE UNE IMAGE, IL DOIT ETRE TRANSFERER DIRECTEMENT DANS UN FILE :
			std::ofstream file("picture.png", std::ofstream::binary | std::ofstream::out);
			file.write(t, bodySize);
		*/
			if (client->getRequest()->getHeaders()["Content-Type"].find("multipart/form-data") != std::string::npos) {
				/* ******trying to upload a file*****************/
				if (strstr(client->getBodyBuf(), "Content-Type") != nullptr 
					&& client->getStatus() == Client::HEADER_PARSED)  {
					std::cout << "+++ENTERING SETPREBODY \n";
					client->setPreBody();
					client->setBodyBuf(client->getBodyBuf() + client->getPreBodySize());
					client->setStatus(Client::PRE_BODY_PARSED);
					client->writeInFile(client->getBodyBuf(), client->getBodyBufSize());
					//parser le prebody dans la request
				}
				else if (client->getStatus() == Client::PRE_BODY_PARSED && n != 0) {
					client->writeInFile(_requestBuffer, n);
				}
				else if (n == 0) {
					// client->getRequest()->parsingBody(); REFAIRE LA FONCTION
					client->setStatus(Client::BODY_PARSED);
					FD_SET(client->getFd(), &_writeSet);
					FD_CLR(client->getFd(), &_readSet);
					//fermer le file;
					if (client->getFd() > _fdMax)
						_fdMax = client->getFd();
				}
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
