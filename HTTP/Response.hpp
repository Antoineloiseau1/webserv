#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "Request.hpp"

class Response
{
	protected:
		Request&							_request;
		std::map<std::string, std::string>	_response;
	public:
		Response(Request &request);
		virtual ~Response();

		virtual void	executor() = 0;

		std::string	openHtmlFile(std::string f);
		std::map<std::string, std::string>	getMap();
};

class GetResponse : public Response
{
	private:
		
	public:
		GetResponse(Request request);
		~GetResponse();

		void	executor();		

};

class PostResponse : public Response
{
	private:
		
	public:
		PostResponse(Request request);
		~PostResponse();

		void	executor();		

};

class DeleteResponse : public Response
{
	private:
		
	public:
		DeleteResponse(Request request);
		~DeleteResponse();

		void	executor();		

};

#endif