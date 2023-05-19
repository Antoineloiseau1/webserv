#include "Client.hpp"

Client::Client(int fd, int serverFd) : _fd(fd), _serverFd(serverFd), _status(0), _request(nullptr),
	_file("picture.png", std::ofstream::binary | std::ofstream::out | std::ofstream::trunc),
	readyForData(false) {
	_status = INIT;
	memset(this->_bodyBuf, 0, BUFFER_SIZE);
		std::cout <<"***********************+++++CLIENT CONSTRUCTOR "<< _fd << "********************\n";

Client::~Client() {}

int	Client::getFd() { return _fd; }

int	Client::getStatus() { return _status; }

void	Client::setStatus(int status) { _status = status; }

std::string	Client::getReqBuf() { return _reqBuf; }

void	Client::addOnReqBuf(std::string buf) {
	_reqBuf += buf;
}

void	Client::createRequest(std::string reqLine) {
	_request = new Request(reqLine);
}

Request	*Client::getRequest() { return _request; }
