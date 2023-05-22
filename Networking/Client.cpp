#include "Client.hpp"
#include <cstring>

Client::Client(int fd, int serverFd) : _fd(fd), _serverFd(serverFd), _status(0), _request(nullptr),
	_file("picture.png", std::ofstream::binary | std::ofstream::out | std::ofstream::trunc),
	readyForData(false) {
	_status = INIT;
	memset(this->_bodyBuf, 0, BUFFER_SIZE);
		std::cout <<"***********************+++++CLIENT CONSTRUCTOR "<< _fd << "********************\n";

}

Client::~Client() {
	std::cout <<"***********************CLIENT DESTRUCTOR "<< _fd << "********************\n";
}

int	Client::getFd() { return _fd; }

int	Client::getStatus() { return _status; }

void	Client::setStatus(int status) { _status = status; }

char	*Client::getBodyBuf() { return _bodyBuf; }

void	Client::createRequest(char *reqLine) {
	_request = new Request(reqLine);
}

void	Client::setBodyBuf(char *buf) { 
	for (int i = 0; i < getBodyBufSize(); ++i) {
        _bodyBuf[i] = buf[i];
    }
}

void	Client::setBodyBufSize(int n) { 
	_bodyBufSize = n;
}

/*A PARSER DANS LA REQUETE DANS UN SECOND TEMPS*/
void	Client::setPreBody() {
	std::string line;
	std::istringstream iss(_bodyBuf);

	getline(iss, line);
	while (!line.empty() && line != "\r") {
		_preBody += line;
		getline(iss, line);
	}
}

void	Client::writeInFile(char *buf, int size) {
	_file.write(buf, sizeof(char) * size);
}

int	Client::getPreBodySize() { return _preBody.size(); }

int	Client::getBodyBufSize() { return _bodyBufSize; }

Request	*Client::getRequest() { return _request; }

int	Client::getServerFd() { return _serverFd; }

std::ofstream	&Client::getFile()  { return _file; }
