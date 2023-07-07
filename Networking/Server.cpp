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
data&			Server::getData() {return _data; }

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
	_alive = false;
}
std::vector<ListeningSocket*>	Server::getSocket(){return _socket;}

void	Server::_watchLoop() {
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
		copy_fd_set(&_readSet, &tmpRead);
		copy_fd_set(&_writeSet, &tmpWrite);
		copy_fd_set(&_errorSet, &tmpError);
		nbEvents = select(_getFdMax() + 1, &tmpRead, &tmpWrite, &tmpError, NULL); //server 0 events are not detected for now (and server 1 events works but handler segfault (?!) )
		if (!nbEvents)
			continue;
		if (i >= _data.getPortsNbr())
			i = 0;
		if (FD_ISSET(_socket[i]->getFd(), &tmpError)) {
			std::cerr << "\nFD_ISSET: " << strerror(errno) << std::endl;
			exit(EXIT_FAILURE);
		}
		if (nbEvents < 1)
			exit(EXIT_FAILURE);
		if(FD_ISSET(_socket[i]->getFd(), &tmpRead) && _alive)
		{
			if (getOpenFd() > MAX_FD)
				_refuse(_socket[i]->getFd());
			else
				_accepter(_socket[i]->getFd(),_socket[i]);
		}

	_clientSizeChange = false;
		for(size_t k = 0; k != _socket[i]->clients.size() && nbEvents-- && _alive; k++)
		{
			if ( _socket[i]->clients.size() == 0)
				break ;
			Client *client = _socket[i]->clients[k];
			if (FD_ISSET(client->getFd(), &tmpError)) {
				std::cerr << "\nFD_ISSET: " << strerror(errno) << std::endl;
				exit(1);
			}
			if(FD_ISSET(client->getFd(), &tmpRead)) {
				if (!_handler(client, i))
					continue;
			}
			if(FD_ISSET(client->getFd(), &tmpWrite))
				_responder(client, i);
			if (_clientSizeChange)
				break ;
		}
		i++;
	}
}

Server::Server(int domain, int service, int protocole, std::vector<int> ports, int nbSocket, char **envp, data& data) : _data(data), _envp(envp), _clientSizeChange(false) {
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
	if (n <= 0) {
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
		if (!client->getRequest())
		{
			disconnectClient(client ,i);
			return 0;
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

	std::cout << "\033[36m#### response from server ####\033[1m\033[94m\n\n"; 
	std::cout << response.getMap()["version"] << response.getMap()["status"];
	std::cout << response.getFile() << "\033[0m" << std::endl;
	std::cout << "\n\033[36m##############################\033[0m\n\n";
	send(client->getFd(), res.c_str(), res.length(), 0);
	disconnectClient(client, i);
}

void	Server::disconnectClient(Client *client, int i) {
	FD_CLR(client->getFd(), &_readSet);
	FD_CLR(client->getFd(), &_writeSet);
	FD_CLR(client->getFd(), &_errorSet);
	close(client->getFd());
	_socket[i]->deleteClient(client->getFd());
	_clientSizeChange = true;

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
	std::cout << "\033[36;41m######## Closing Server ######### \n";
	std::cout << "\033[0m";
	for (std::vector<std::string>::iterator it = pictPaths.begin(); it != pictPaths.end(); it++) {
		if (std::remove((*it).c_str()) != 0)
			std::cerr << "Failed to delete file: " << *it << std::endl;
		else
			std::cout << "File deleted successfully" << std::endl;
	}
	for (int i = 0; i != _data.getPortsNbr(); i++)
		delete this->_socket[i]; 
}


void	Server::changeDupName(std::string &file_name) {
	std::string	ext = file_name.substr(file_name.find_last_of('.'), file_name.size() - file_name.find_last_of('.'));
	std::string	name = file_name.substr(0, file_name.find_last_of('.'));

	file_name = name + "1" + ext;
}


void	Server::checkForDupName(std::string &file_name) {
	for (std::vector<std::string>::iterator it = pictPaths.begin(); it != pictPaths.end(); it++) {
		if (*it == "data/images/" + file_name) {
			changeDupName(file_name);
			it = pictPaths.begin();
		}
	}
}

std::string	Server::addPicture(std::string file_name) {
	checkForDupName(file_name);
	file_name = urlEncode(file_name);
	pictPaths.push_back("data/images/" + file_name);
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
