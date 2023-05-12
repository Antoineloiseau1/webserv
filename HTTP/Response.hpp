#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "Request.hpp"
#include "../Networking/Server.hpp"

class	Server;

class Response
{
	protected:

		Server&								_server;
		Request&							_request;
		std::map<std::string, std::string>	_response;


	public:

		Response(Request &request, Server &server);
		virtual ~Response();

		void				executor();
		std::string			buildResponse(void);

		std::string	openHtmlFile(std::string f);
		std::map<std::string, std::string>	getMap();
		void								handleCgi();
		void								GetResponse();
		void								PostResponse();
		void								DeleteResponse();
		void								NotImplemented();
		void								BadRequestError();

};

#endif