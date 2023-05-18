#include "Client.hpp"
#include <cstring>

Client::Client(int fd, int serverFd) : _fd(fd), _serverFd(serverFd), _status(0), _request(nullptr) {
	_status = INIT;
	memset(this->_reqBuf, 0, BUFFER_SIZE);
		std::cout <<"***********************+++++CLIENT CONSTRUCTOR "<< _fd << "********************\n";

}

Client::~Client() {
	std::cout <<"***********************CLIENT DESTRUCTOR "<< _fd << "********************\n";
}

int	Client::getFd() { return _fd; }

int	Client::getStatus() { return _status; }

void	Client::setStatus(int status) { _status = status; }

std::string	Client::getReqBuf() { return _reqBuf; }

void	Client::createRequest(char *reqLine) {
	_request = new Request(reqLine);
}

void	Client::setReqBuf(char *buf) { 
	_reqBuf = buf; /*A MODIFIER ! COPIE PROFONDE A FAIRE*/
}

Request	*Client::getRequest() { return _request; }

int	Client::getServerFd() { return _serverFd; }
