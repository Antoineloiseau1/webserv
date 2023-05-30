#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "Request.hpp"
#include "../Networking/Server.hpp"
#include <array>
#include <map>
#include <vector>

class	Server;

class Response
{
	protected:

		Server&								_server;
		Request&							_request;
		std::map<std::string, std::string>	_response;
		char **envp;
		std::array<std::string, 6>			_env;
		std::string 						_tmpPictFile;
		size_t								_contentSize;


	public:

		Response(Request &request, Server &server, std::string tmp_file);
		virtual ~Response();

		void				executor();
		std::string			buildResponse(void);

		std::string	openHtmlFile(std::string f);
		std::map<std::string, std::string>	getMap();
		void								createCgiEnv();
		void								handleCgi(std::string file);
		void								GetResponse();
		void								PostResponse();
		void								DeleteResponse();
		void								NotImplemented();
		void								BadRequestError();

		void								fillGetBody(std::string file);
		void								fillGetLength();
		void								fillGetType(std::string file);
};

#endif
