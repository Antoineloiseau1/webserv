#include "Server.hpp"
#include <exception>
#include <csignal>

#include "../parsing/parsing.hpp"
#include "../HTTP/Response.hpp"
#include "Client.hpp"

#define MAX_FD 230

bool	_alive = true;

#include <unistd.h>

static void ft_putstr(const char* str) {
    size_t i = 0;
    while (str[i] != '\0') {
        write(1, &str[i], 1);
        i++;
    }
}

void copy_fd_set(fd_set* src, fd_set* dest) {
    FD_ZERO(dest);  // Clear the destination fd_set
    
    int fd;
    for (fd = 0; fd < FD_SETSIZE; ++fd) {
        if (FD_ISSET(fd, src)) {
            FD_SET(fd, dest);  // Copy the file descriptor to the destination fd_set
        }
    }
}

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
		copy_fd_set(&_readSet, &tmpRead);
		copy_fd_set(&_writeSet, &tmpWrite);
		copy_fd_set(&_errorSet, &tmpError);
		// tmpRead = _readSet;
		// tmpWrite = _writeSet;
		// tmpError = _errorSet;
		// std::cout << i << "whille\n";
		nbEvents = select(_getFdMax() + 1, &tmpRead, &tmpWrite, &tmpError, NULL);

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
		for(size_t k = 0; k != _socket[i]->clients.size() && nbEvents-- && _alive; k++) //hate u
		{
			if (j >= _socket[i]->clients.size()) //may not be necessary anymore but i'm scared to delete it
				break;
			std::cout << "size : " << _socket[i]->clients.size() << std::endl;
			std::cout<< "heap buffer overflow " << j++ << std::endl;
			std::cout << i << "nbevnts "<< nbEvents << std::endl;
			Client *client = _socket[i]->clients[k];
			if (FD_ISSET(client->getFd(), &tmpError)) {
				std::cout << "ERROR SET CLIENT\n";
				exit(1);
			}
			if(FD_ISSET(client->getFd(), &tmpRead)) {
				if (!_handler(client, i)){
					i++;
					continue ;}
			}
			if(FD_ISSET(client->getFd(), &tmpWrite)) {
				_responder(client, i);
			}
		}
		i++;
	}
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

/* A METTRE DANS LES UTILS*/
std::string	create_tmp_file_name(int fd) {
	std::ostringstream oss;
    oss << "picture" << fd << ".png";
    std::string s = oss.str();
	return s;
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
	Client* newClient = new Client(fd, sock->getFd(), create_tmp_file_name(fd));
	sock->setClient(newClient);
}

/*A SUPPRIMER: NE SERT A RIEN ?*/
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
int	Server::_handler(Client *client, int i) {

	int	n = recv(client->getFd(), _requestBuffer, BUFFER_SIZE, 0);
	std::cout << "***** n = " << n << std::endl;
	if (n <= 0) { // mettre <=0 et gerer proprement pour la boucle infinie
		std::cerr << "error: recv: " << strerror(errno) << std::endl;
		disconnectClient(client, i);
		return 0;
	}
	else {
		_requestBuffer[n] = '\0';
		std::cout << "requestBuffer = \n" ;
		ft_putstr(_requestBuffer);
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
							if (client->bytes > 0) {
								std::cout << "JE SUIS POPOPOP: request buffer \n" ;
								ft_putstr(_requestBuffer);
								ft_putstr("FIN \n");
								if (client->parsePreBody(_requestBuffer + client->getRequest()->getHeaderLen(), client->bytes))
									client->getRequest()->setFileName(addPicture(client->getRequest()->getFileName()));
								std::cout << "APRES Request buffer: \n" ;
								ft_putstr(_requestBuffer);
								ft_putstr("FIN \n");
								n = 0;
							}
						}
						else if (client->getStatus() == Client::PARSING_PREBODY){
							std::cout << "JE SUIS LALALAL \n" ;
							if (client->parsePreBody(_requestBuffer, n))
								client->getRequest()->setFileName(addPicture(client->getRequest()->getFileName()));
							n = 0;
						}
					}
					break;
				case Client::POST_FORM:
					if (client->getStatus() < Client::PARSING_PREBODY) {
						client->bytes = n - client->getRequest()->getHeaderLen();
						client->setStatus(Client::PARSING_PREBODY);
						if (client->bytes > 0) {
							client->setFormBody(_requestBuffer + client->getRequest()->getHeaderLen());
						}
					}
					else if (client->getStatus() == Client::PARSING_PREBODY){
							client->setFormBody(_requestBuffer);
					}
					break;
			}
		}
		if (client->bytes >= atoi(client->getRequest()->getHeaders()["Content-Length"].c_str())) {
				std::cout << "++++BYTES = "<< client->bytes << " | atoi = " << atoi(client->getRequest()->getHeaders()["Content-Length"].c_str()) << std::endl;
				client->setStatus(Client::BODY_PARSED); //ne sert surement a rien
				client->writeInFile(_requestBuffer, n);
				std::cout << "++++Request buffer FINAL: \n" ;
				ft_putstr(_requestBuffer);
				setToWrite(client);
				client->getRequest()->parseFormBody(client->getFormBody());
				client->bytes = 0;
				client->getFile().close(); //closing file after finishing to write data
		}
		else if (client->getStatus() == Client::READY_FOR_DATA && n > 0) {
			std::cout << "+++++++++++++++j'ecris dans le file ... N = " << n << std::endl;
				client->writeInFile(_requestBuffer, n);
			}
	}
	return 1;
}

void	Server::_responder(Client *client, int i) {
	
	Response	response(*(client->getRequest()), *this, client->getTmpPictFile());
	std::string res = response.buildResponse();

	std::cout << "Response from the server:\n" << res << std::endl;
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
	for (std::vector<std::string>::iterator it = pictPaths.begin(); it != pictPaths.end(); it++) {
		if (std::remove((*it).c_str()) != 0) {
			std::cerr << "Failed to delete file: " << *it << std::endl;
		} else {
			std::cout << "File deleted successfully" << std::endl;
		}
	}
	for (int i = 0; i != _data.getPortsNbr(); i++)
		delete this->_socket[i]; }


void	Server::changeDupName(std::string &file_name) {
	std::string	ext = file_name.substr(file_name.find_last_of('.'), file_name.size() - file_name.find_last_of('.'));
	std::string	name = file_name.substr(0, file_name.find_last_of('.'));

	file_name = name + "1" + ext;
}


void	Server::checkForDupName(std::string &file_name) {
	for (std::vector<std::string>::iterator it = pictPaths.begin(); it != pictPaths.end(); it++) {
		if (*it == "uploads/" + file_name) {
			changeDupName(file_name);
			it = pictPaths.begin();
		}
	}
}

std::string	Server::addPicture(std::string file_name) {
	checkForDupName(file_name);
	pictPaths.push_back("uploads/" + file_name);
	return file_name;
}

void	Server::deletePict(std::string path) {
	for (std::vector<std::string>::iterator it = pictPaths.begin(); it != pictPaths.end(); it++) {
		if (path == *it) {
			pictPaths.erase(it);
			break;
		}
	}
}
