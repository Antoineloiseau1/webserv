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
		std::string							_file;
		char 								**envp;
		std::array<std::string, 6>			_env;
		std::string 						_tmpPictFile;
		size_t								_contentSize;


	public:

		Response(Request &request, Server &server, std::string tmp_file, int fd);
		virtual ~Response();

		std::string			buildResponse(void);

		std::string	openHtmlFile(std::string f);
		std::map<std::string, std::string>	getMap();
		std::string							getFile();
		void								createCgiEnv();
		void								handleCgi(std::string file, int fd);
		void								GetResponse(int fd);
		void								PostResponse(int fd);
		void								DeleteResponse();
		void								NotImplemented();
		void								BadRequestError();
		std::string							findRoute(std::string const file);
		int									findServer();
		int									checkPermissions(const char *directory, std::string file);

		void								notFound404(void);
		void								forbidden403(void);
		void								ok200(void);
		void								noContent204(void);
		void								internalServerError505(void);

		void								fillGetBody(std::string file);
		void								fillGetLength();
		void								fillGetType(std::string file);
};

#endif
