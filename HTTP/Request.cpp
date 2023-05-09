#include "Request.hpp"

Request::Request(std::string request) {
	
	std::string result;
	std::istringstream iss(request);
	
	std::getline(iss, result);
	std::istringstream first_line(result);

	first_line >> this->_initialRequestLine["type"];
	first_line >> this->_initialRequestLine["path"];
	this->_initialRequestLine["path"].erase(0, 1);
	first_line >> this->_initialRequestLine["version"];
	if (request.find("nom=") != std::string::npos)
		_body = request.substr(request.find("nom="), request.size() - request.find("nom="));
}

std::string	Request::getType() { return _initialRequestLine["type"]; }

std::string	Request::getPath() { return _initialRequestLine["path"]; }

std::string	Request::getVersion() { return _initialRequestLine["version"]; }

std::string	Request::getBody() { return _body; }