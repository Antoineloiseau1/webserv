#include "Client.hpp"
#include <cstring>
#include <unistd.h>

static void ft_putstr(const char* str) {
    size_t i = 0;
    while (str[i] != '\0') {
        write(1, &str[i], 1);
        i++;
    }
}


Client::Client(int fd, int serverFd, std::string tmp_file) : _fd(fd), _serverFd(serverFd), _status(0), _request(nullptr),
	_tmpPictFile(tmp_file), _file(tmp_file, std::ofstream::binary | std::ofstream::out | std::ofstream::trunc),
	readyForData(false), bytes(0) {
		// std::cout << "PRINT TMP FILE = "<< tmp_file << std::endl;
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

void	Client::writeInFile(char *buf, int size) {
	_file.write(buf, sizeof(char) * size);
}

int	Client::parsePreBody(char *buf, int size) {
	std::cout << "-Pre body initial = " << _preBody << std::endl ;
	_preBody += buf;
	if (_preBody.find("Content-Type") != std::string::npos) {
		std::cout << "---je me fais parser le pre body : size = " << size << "_ buf =" << buf << std::endl ;
		_request->parsingPreBody(_preBody);
		writeInFile(buf + _request->getPreBody().size() + 5, size - _request->getPreBody().size());
		std::cout << "PREBODY SIZE = " << _request->getPreBody().size() << "|" << size - _request->getPreBody().size() << std::endl;
		ft_putstr(_preBody.c_str());
		ft_putstr("|FIN \n");
		ft_putstr(buf + _request->getPreBody().size() + 5);
		ft_putstr("FIN \n");
		setStatus(Client::READY_FOR_DATA);
		return 1;
	}
	return 0;
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

std::string	Client::getTmpPictFile() { return _tmpPictFile; }
