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
		std::string							_preBody;
		std::string							_formBody;
		std::string							_fileName;
		std::string							_fileToDelete;
		std::map<std::string, std::string>	_formInfo;
		
	public:
		Request(char *request);
		~Request(void);
		
		bool								isADataUpload;
		bool								isDelete;	

		std::string							getTypeStr();
		std::string							getPath();
		std::string							getVersion();
		std::map<std::string, std::string>	getHeaders();
		std::string							getBody();
		std::string							getPreBody();
		std::string							getFileName();
		int									getHeaderLen();
		std::string							getFileToDelete();

		void								parsingPreBody(std::string	pre_body);
		void								separateHeaders(std::string reqString);
		void								parseFormBody(std::string body);
		void								setFileName(std::string new_name);
		
};



#endif