#include "Request.hpp"
#include <fstream>


void	Request::separateHeaders(std::string reqString)
{
	_headerLine = reqString.substr(0, reqString.find("\r\n\r\n") + 4);
}

/* Parses only the headers */
Request::Request(char *requestBuf) : isADataUpload(false), isDelete(false) {
	_requestLine = requestBuf;
	separateHeaders(requestBuf);
	std::string line;
	std::istringstream iss(_headerLine);

	std::getline(iss, line);
	while(!line.empty() && line == "\r")
		getline(iss, line);
	std::istringstream first_line(line);
	first_line >> this->_initialRequestLine["type"];
	first_line >> this->_initialRequestLine["path"];
	this->_initialRequestLine["path"].erase(0, 1);
	first_line >> this->_initialRequestLine["version"];
	getline(iss, line);
	while(!line.empty() && line != "\r")
	{
		int delim = line.find_first_of(':');
		_headers[line.substr(0, delim)] = line.substr(delim + 2, line.size() - (delim + 2));
		getline(iss, line);
	}
}

void	Request::parsingPreBody(std::string	pre_body) {
	std::string line;
	std::istringstream iss(pre_body);
	getline(iss, line);
	while (!line.empty() && line != "\r") {
		_preBody += line;
		if (line.find("filename=") != std::string::npos)
			_fileName = line.substr(line.find("filename=") + 10, line.size() - (line.find("filename=") + 12));
		line.clear();
		getline(iss, line);
	}
}

/* A METTRE DANS UTILS*/
std::string urlDecode(const std::string& encoded) {
    std::string decoded;
   size_t i = 0;

    while (i < encoded.length()) {
        if (encoded[i] == '%') {
            int hexValue;
            sscanf(encoded.substr(i + 1, 2).c_str(), "%x", &hexValue);
            decoded += static_cast<char>(hexValue);
            i += 3;
        } else {
            decoded += encoded[i];
            ++i;
        }
    }
    return decoded;
}

Request::~Request(void) {}

std::string	Request::getTypeStr() { return _initialRequestLine["type"]; }

std::string	Request::getPath() { return _initialRequestLine["path"]; }

std::string	Request::getVersion() { return _initialRequestLine["version"]; }

std::string	Request::getBody() { return _body; }

std::string	Request::getPreBody() { return _preBody; }

std::map<std::string, std::string>	Request::getHeaders() { return _headers; }

int	Request::getHeaderLen() { return _headerLine.size(); }

std::string	Request::getFileName() { return _fileName; }

void	Request::setFileName(std::string new_name) {
	_fileName = new_name;
}

std::string	Request::getFileToDelete() { return _fileToDelete; } 

/*
image=%2Fuploads%2Fimagetest.jpg -> path extracted
nom=Demma&prenom=Manon&email=manon.demma%40gmail.com&ville=Nice -> in a map
*/
void	Request::parseFormBody(std::string body) {
	_formBody = body;
	if (!_formBody.empty()) {
		if (isDelete) {
			int loc = _formBody.find('=') + 1;
			_fileToDelete = urlDecode(_formBody.substr(loc, _formBody.size() - loc));
		}
		else {
			std::stringstream ss(_formBody);
			std::string token;
    		while (std::getline(ss, token, '&')) {
				int loc = token.find('=');
        		_formInfo[token.substr(0, loc)] = urlDecode(token.substr(loc + 1, token.size() - loc));
			}
		}
	}	
}