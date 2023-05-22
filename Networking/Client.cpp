#include "Client.hpp"
#include <cstring>
#include <unistd.h>

Client::Client(int fd, int serverFd) : _fd(fd), _serverFd(serverFd), _status(0), _request(nullptr),
	_file("picture.png", std::ofstream::binary | std::ofstream::out | std::ofstream::trunc),
	readyForData(false), bytes(0) {
	_status = INIT;
		std::cout <<"***********************+++++CLIENT CONSTRUCTOR "<< _fd << "********************\n";

}

Client::~Client() {
	std::cout <<"***********************CLIENT DESTRUCTOR "<< _fd << "********************\n";
}

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
	else if (_request->getTypeStr() == "GET" || _request->getTypeStr() == "DELETE")
		_type = GET_DELETE;
}

void	Client::writeInFile(char *buf, int size) {
	_file.write(buf, sizeof(char) * size);
}

void	Client::parsePreBody(char *buf, int size) {
	_preBody += buf;
	std::cout << "PRE BODY TEST = " << _preBody << " | buf = " << buf << std::endl;
	if (_preBody.find("Content-Type") != std::string::npos) {
		_request->parsingPreBody(_preBody);
		writeInFile(buf + _request->getPreBody().size() + 5, size - _request->getPreBody().size());
		std::cout <<" SIZE WRITTEN = " << _request->getPreBody().size() << std::endl;
		setStatus(Client::READY_FOR_DATA);
	}
}

void	Client::setFormBody(std::string buf) {
	_formBody += buf;
}

/********************** GETTERS **************************/

int	Client::getFd() { return _fd; }

int	Client::getStatus() { return _status; }

void	Client::setStatus(int status) { _status = status; }

Request	*Client::getRequest() { return _request; }

int	Client::getServerFd() { return _serverFd; }

std::ofstream	&Client::getFile()  { return _file; }

int	Client::getType() { return _type; }

std::string	Client::getFormBody() { return _formBody; }
