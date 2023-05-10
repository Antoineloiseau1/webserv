#ifndef REQUEST_HPP
# define REQUEST_HPP

#include <map>
#include <string>
#include <iostream>
#include <sstream>

class Request {
	private:
		std::map<std::string, std::string>	_initialRequestLine;
		std::string							_body;
		// std::map<std::string, std::string>	_headerLines;
	public:
		Request(std::string request);
		~Request(void) {};

		std::string	getType();
		std::string	getPath();
		std::string	getVersion();
		std::string	getBody();
};



#endif