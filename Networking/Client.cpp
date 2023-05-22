#include "Client.hpp"
#include <cstring>
#include <unistd.h>

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
	if (_request->getTypeStr() == "POST") {
		if (_request->getHeaders()["Content-Type"].find("multipart/form-data") != std::string::npos) {
			_request->isADataUpload = true;
			_type = POST_DATA;
		}
		else if (_request->getHeaders()["Content-Type"].find("application/x-www-form-urlencoded") != std::string::npos)
			_type = POST_FORM;
	}
	else if (_request->getTypeStr() == "GET")
		_type = GET;
	else if (_request->getTypeStr() == "DELETE")
		_type = DELETE;
}

void	Client::setBodyBuf(char *buf) { 
	for (int i = 0; i < getBodyBufSize(); ++i) {
        _bodyBuf[i] = buf[i];
    }
	std::cout << "GET BODY BUF SIZE !!!! " << getBodyBufSize() <<std::endl;
	// write(1, "\n\n***WRITING**\n\n", 16);
	// for (int i = 0; i <  getBodyBufSize(); i++)
	// 	write(1, &buf[i], 1);
	// write(1, "\n\n***WRITING END**\n\n", 20);
}

void	Client::addOnBodyBuf(char *buf, int size) { 
	int i = 0;

	while (i < size) {
        _bodyBuf[_bodyBufSize + i] = buf[i];
		i++;
    }
	_bodyBufSize += size;
	_bodyBuf[_bodyBufSize + i] = '\0';
}

void	Client::setBodyBufSize(int n) { 
	_bodyBufSize = n;
}

/*A PARSER DANS LA REQUETE DANS UN SECOND TEMPS*/
// void	Client::setPreBody(std::string pre_body) {
// 	std::string line;
// 	std::istringstream iss(pre_body);

// 	getline(iss, line);
// 	while (!line.empty() && line != "\r") {
// 		_preBody += line;
// 		getline(iss, line);
// 	}
// }

void	Client::writeInFile(char *buf, int size) {
	_file.write(buf, sizeof(char) * size);
}

void	Client::parsePreBody(char *buf, int size) {
	_preBody += buf;
	std::cout << "PRE BODY TEST = " << _preBody << " | buf = " << buf << std::endl;
	if (_preBody.find("Content-Type") != std::string::npos) {
		_request->parsingPreBody(_preBody);
		setBodyBufSize(size - _request->getPreBody().size());
		setBodyBuf(buf + _request->getPreBody().size() + 5); //on cut le prebody du body restant
		writeInFile(_bodyBuf, size - _request->getPreBody().size());
		std::cout <<" SIZE WRITTEN = " << _request->getPreBody().size() << std::endl;
		setStatus(Client::READY_FOR_DATA);
	}
}

int	Client::getPreBodySize() { return _preBody.size(); }

int	Client::getBodyBufSize() { return _bodyBufSize; }

Request	*Client::getRequest() { return _request; }

int	Client::getServerFd() { return _serverFd; }

std::ofstream	&Client::getFile()  { return _file; }

int	Client::getType() { return _type; }
