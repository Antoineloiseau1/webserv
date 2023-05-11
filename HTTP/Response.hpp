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

		virtual void		executor() = 0;
		std::string			buildResponse(void);

		std::string	openHtmlFile(std::string f);
		std::map<std::string, std::string>	getMap();
		void								handleCgi();
};

class GetResponse : public Response
{
	private:
		
	public:
		GetResponse(Request request, Server &server);
		~GetResponse();

		void	executor();		

};

class PostResponse : public Response
{
	private:
		
	public:
		PostResponse(Request request, Server &server);
		~PostResponse();

		void	executor();		

};

class DeleteResponse : public Response
{
	private:
		
	public:
		DeleteResponse(Request request, Server &server);
		~DeleteResponse();

		void	executor();		

};

class BadRequestError: public Response {

	public:
		BadRequestError(Request request, Server& server);
		~BadRequestError(void);

		void	executor();
};

#endif