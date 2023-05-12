#ifndef REQUEST_HPP
# define REQUEST_HPP

#include <map>
#include <string>
#include <iostream>
#include <sstream>

class Request {
	private:
		std::map<std::string, std::string>	_initialRequestLine;
		std::map<std::string, std::string>	_headers;
		std::string							_body;
	public:
		Request(std::string request);
		~Request(void);

		std::string							getType();
		std::string							getPath();
		std::string							getVersion();
		std::string							getBody();
		std::map<std::string, std::string>	getHeaders();
};



#endif