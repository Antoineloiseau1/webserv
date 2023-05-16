#include "Client.hpp"

Client::Client(int fd) : _fd(fd), _status(0), _request(nullptr) {}

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
