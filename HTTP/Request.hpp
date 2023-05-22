#ifndef REQUEST_HPP
# define REQUEST_HPP

#include <map>
#include <string>
#include <iostream>
#include <sstream>

class Request {
	private:
		std::string							_requestLine;
		std::string							_headerLine;
		std::map<std::string, std::string>	_initialRequestLine;
		std::map<std::string, std::string>	_headers;
		std::string							_body;

	public:
		Request(char *request);
		~Request(void);

		
		bool								isADataUpload;

		std::string							getType();
		std::string							getPath();
		std::string							getVersion();
		std::map<std::string, std::string>	getHeaders();
		std::string							getBody();
		int									getHeaderLen();

		void								parsingBody();
		void								separateHeaders(std::string reqString);
		
};



#endif