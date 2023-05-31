#include "Client.hpp"
#include <cstring>
#include <unistd.h>



Client::Client(int fd, int serverFd, std::string tmp_file) : _fd(fd), _serverFd(serverFd), _status(0), _request(nullptr),
	_tmpPictFile(tmp_file), _file(tmp_file, std::ofstream::binary | std::ofstream::out | std::ofstream::trunc), readyForData(false), bytes(0) {
		// std::cout << "PRINT TMP FILE = "<< tmp_file << std::endl;
	_status = INIT;
	std::cout << std::endl << "****** New connexion on server from client fd: "<< _fd << " *******\n";
}

Client::~Client() {
	std::cout << "***** Closing connexion of client: " << _fd << " ******\n";
	delete (_request);
}

void	Client::createRequest(char *reqLine) {
	_request = new Request(reqLine);
	if (_request->getTypeStr() == "POST") {
		if (_request->getHeaders()["Content-Type"].find("multipart/form-data") != std::string::npos) {
			_request->isADataUpload = true;
			_type = POST_DATA;
		}
		else if (_request->getHeaders()["Content-Type"].find("application/x-www-form-urlencoded") != std::string::npos) {
			_type = POST_FORM;
			if (_request->getPath() == "delete")
				_request->isDelete = true;
			if (_request->getHeaders().find("Transfer-Encoding") != _request->getHeaders().end()
				&& _request->getHeaders()["Transfer-Encoding"].find("chunked") != std::string::npos)
				_request->isChunked = true;
		}
	}
	else if (_request->getTypeStr() == "GET" || _request->getTypeStr() == "DELETE")
		_type = GET_DELETE;
	if (_type != POST_DATA) {
		int result = std::remove(_tmpPictFile.c_str());
		if (result == 0)
			std::cout << "File deleted successfully.\n";
		else
			std::cout << "Failed to delete the file.\n";
	}
}

void	Client::writeInFile(char *buf, int size) { _file.write(buf, sizeof(char) * size); }

int	Client::parsePreBody(char *buf, int size) {
	_preBody += buf;
	if (_preBody.find("Content-Type") != std::string::npos) {
		_request->parsingPreBody(_preBody);
		writeInFile(buf + _request->getPreBody().size() + 5, size - _request->getPreBody().size());
		setStatus(Client::READY_FOR_DATA);
		return 1;
	}
	return 0;
}

void	Client::setFormBody(std::string buf) { _formBody += buf; }


/********************** GETTERS **************************/

int	Client::getFd() { return _fd; }

int	Client::getStatus() { return _status; }

void	Client::setStatus(int status) { _status = status; }

Request	*Client::getRequest() { return _request; }

int	Client::getServerFd() { return _serverFd; }

std::ofstream	&Client::getFile()  { return _file; }

int	Client::getType() { return _type; }

std::string	Client::getFormBody() { return _formBody; }

std::string	Client::getTmpPictFile() { return _tmpPictFile; }
