#include "Request.hpp"

Request::Request(std::string request) {
	
	std::string line;
	std::istringstream iss(request);

	std::getline(iss, line);
	// Ignore whitepaces
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
	getline(iss, _body);
}

Request::~Request(void) {}

std::string	Request::getType() { return _initialRequestLine["type"]; }

std::string	Request::getPath() { return _initialRequestLine["path"]; }

std::string	Request::getVersion() { return _initialRequestLine["version"]; }

std::string	Request::getBody() { return _body; }

std::map<std::string, std::string>	Request::getHeaders() {return _headers; };
