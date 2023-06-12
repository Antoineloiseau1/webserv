#include "Server.hpp"
#include <exception>
#include <csignal>

#include "../parsing/parsing.hpp"
#include "../HTTP/Response.hpp"
#include "Client.hpp"

#define MAX_FD 230

bool	_alive = true;

#include <unistd.h>

void copy_fd_set(fd_set* src, fd_set* dest) {
    FD_ZERO(dest);  // Clear the destination fd_set
    
    int fd;
    for (fd = 0; fd < FD_SETSIZE; ++fd) {
        if (FD_ISSET(fd, src)) {
            FD_SET(fd, dest);  // Copy the file descriptor to the destination fd_set
        }
    }
}
data			Server::getData() const {return _data; }

int	Server::getRequestFd() const { return _requestFd; }

char	**Server::getEnvp() const { return _envp; }

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
std::vector<ListeningSocket*>	Server::getSocket(){return _socket;}

void _watchLoop(std::vector<Server *> servers) {
	int 	nbEvents;
	int	i = 0;
	size_t nbr;

	fd_set	tmpRead;
	fd_set	tmpWrite;
	fd_set	tmpError;
	FD_ZERO(&tmpRead);
	FD_ZERO(&tmpWrite);
	FD_ZERO(&tmpError);
	signal(SIGINT, exit);

	while(_alive) 
	{
		std::cout << " server size : " << servers.size() << std::endl;
		for (nbr = 0; nbr != servers.size(); nbr++)
		{
			copy_fd_set(&(servers[nbr]->_readSet), &tmpRead);
			copy_fd_set(&(servers[nbr]->_writeSet), &tmpWrite);
			copy_fd_set(&(servers[nbr]->_errorSet), &tmpError);
		}
			std::cout << "before select\n";

			nbEvents = select(servers[servers.size() - 1]->_getFdMax() + 1, &tmpRead, &tmpWrite, &tmpError, NULL); //server 0 events are not detected for now (and server 1 events works but handler segfault (?!) )
			std::cout << "nbevents : " << nbEvents << std::endl;
			if (!nbEvents)
				continue;
			std::cout << "after select\n";

		for (nbr = 0; nbr != servers.size(); nbr++)
		{
			if (i >= servers[nbr]->getData().getPortsNbr(nbr))
				i = 0;
			std::cout << "SERVER " << nbr << std::endl;
			if (FD_ISSET(servers[nbr]->getSocket()[i]->getFd(), &tmpError)) {
				std::cerr << "ERROR SET SERVER\n";
				exit(EXIT_FAILURE);
			}
			if (nbEvents < 1) {
				std::cerr << "Error: select(): " << strerror(errno) << std::endl;
				exit(EXIT_FAILURE);
			}
			std::cout << "Server " << nbr<< " ready to work" << std::endl;
			std::cout << "is read set ? " << FD_ISSET(servers[nbr]->getSocket()[i]->getFd(), &tmpRead) << std::endl; //problem is that only the last server is set 
			if(FD_ISSET(servers[nbr]->getSocket()[i]->getFd(), &tmpRead) && _alive)
			{
				std::cout << "Server " << nbr << "is set\n";
				if (servers[nbr]->getOpenFd() > MAX_FD)
					servers[nbr]->_refuse(servers[nbr]->getSocket()[i]->getFd());
				else
				{
					std::cout << "accepter\n\n";
					servers[nbr]->_accepter(servers[nbr]->getSocket()[i]->getFd(),servers[nbr]->getSocket()[i]);
				}
			}

			for(size_t k = 0; k != servers[nbr]->getSocket()[i]->clients.size() && nbEvents-- && _alive; k++) //hate u
			{
				std::cout << "FOR LOOP \n";
				std::cout << "Server " << nbr << " is in for loop\n";
				Client *client = servers[nbr]->getSocket()[i]->clients[k];
				if (FD_ISSET(client->getFd(), &tmpError)) {
					std::cout << "ERROR SET CLIENT\n";
					exit(1);
				}
				if(FD_ISSET(client->getFd(), &tmpRead)) {
					std::cout << "handler\n\n";
					if (!servers[nbr]->_handler(client, i))
						continue;
				}
				if(FD_ISSET(client->getFd(), &tmpWrite))
				{
					std::cout << "responder\n\n";
					servers[nbr]->_responder(client, i);
				}	
			}
			i++;
		}
	}
}

Server::Server(int domain, int service, int protocole, int *ports, int nbSocket, char **envp, data& data) : _data(data), _envp(envp) {
	std::cout << "\033[30;42m";
	std::cout << "### Server is now open ###\033[0m" << std::endl << std::endl;

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
		std::cout << "setting " << socket->getFd() << std::endl;
		FD_SET(socket->getFd(), &_readSet);
		FD_SET(socket->getFd(), &_errorSet);
		if(socket->getFd() > _fdMax)
			_fdMax = socket->getFd();
	}
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
	if (n <= 0) {
		std::cerr << "error: recv: " << strerror(errno) << std::endl;
		disconnectClient(client, i);
		return 0;
	}
	else {
		_requestBuffer[n] = '\0';
		/*******************POUR TOUT LE MONDE 1 X*****************************/
		if (strstr(_requestBuffer, "\r\n\r\n") && client->getStatus() == Client::INIT) {
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
								if (client->parsePreBody(_requestBuffer + client->getRequest()->getHeaderLen(), client->bytes))
									client->getRequest()->setFileName(addPicture(client->getRequest()->getFileName()));
								n = 0;
							}
						}
						else if (client->getStatus() == Client::PARSING_PREBODY)
						{
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
						if (client->bytes > 0)
							client->setFormBody(_requestBuffer + client->getRequest()->getHeaderLen());
					}
					else if (client->getStatus() == Client::PARSING_PREBODY){
							client->setFormBody(_requestBuffer);
					}
					break;
			}
		}
		if (client->bytes >= atoi(client->getRequest()->getHeaders()["Content-Length"].c_str())) {
				client->writeInFile(_requestBuffer, n);
				setToWrite(client);
				client->getRequest()->parseFormBody(client->getFormBody());
				client->bytes = 0;
				client->getFile().close(); //closing file after finishing to write data
		}
		else if (client->getStatus() == Client::READY_FOR_DATA && n > 0) {
				client->writeInFile(_requestBuffer, n);
			}
	}
	return 1;
}

void	Server::_responder(Client *client, int i) {
	Response	response(*(client->getRequest()), *this, client->getTmpPictFile(), client->getFd());
	std::string res = response.buildResponse();

	std::cout << "\033[36m";
	std::cout << "#### response from server:\033[1m\033[94m" << response.getMap()["status"].substr(0, response.getMap()["status"].length() - 2);
	std::cout << " " << response.getFile() << "\033[0m" << std::endl;
	send(client->getFd(), res.c_str(), res.length(), 0);
	disconnectClient(client, i);
}

void	Server::disconnectClient(Client *client, int i) {
	FD_CLR(client->getFd(), &_readSet);
	FD_CLR(client->getFd(), &_writeSet);
	FD_CLR(client->getFd(), &_errorSet);
	close(client->getFd());
	_socket[i]->deleteClient(client->getFd());
}

ListeningSocket	*Server::getSocket(int fd) {
	for (std::vector<ListeningSocket*>::iterator it = _socket.begin(); it != _socket.end(); it++)
	{
		if ((*it)->getFd() == fd)
			return *it;
	}
	return nullptr;
}

Server::~Server(void) {
	std::cout << "\n\n \033[36;41m######## Closing Server ######### \n";
	std::cout << "\033[0m";
	for (std::vector<std::string>::iterator it = pictPaths.begin(); it != pictPaths.end(); it++) {
		if (std::remove((*it).c_str()) != 0)
			std::cerr << "Failed to delete file: " << *it << std::endl;
		else
			std::cout << "File deleted successfully" << std::endl;
	}
//	for (int i = 0; i != _data.getPortsNbr(); i++)
//		delete this->_socket[i]; 
}


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
