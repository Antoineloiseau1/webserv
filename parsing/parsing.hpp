#pragma once

#include <map>
#include <vector>
#include <iostream>
#include <exception>
#include <sstream>

class data
{
	//vector[int:server][string:route][string:nomparam][string:valueparam]
	private:
		std::vector<std::map<std::string, std::map<std::string, std::string> > >	_servers;
		std::string							_name;

		std::vector<std::string>			_possibleSettings;
		std::vector<std::string>			_routeSettings;
		std::vector<std::string>			_onlyRouteSettings;

		std::vector<std::string>			_routes;
		void								fill(std::fstream &file, std::string route);
		std::vector<int>					_ports;
		int									_portsNbr;
		std::string							_customErrPage;
		std::vector<std::string>			_customErrors;
		std::vector<std::string>			_indexFiles;

	public:
		data(std::string conf);
		~data();

		void								makePorts(size_t i);
		void								areServersDifferent();
		void								setSettings();
		std::string							whichSetting(std::string line);

		int									isRoute;
		void								newRouteSetup(std::string &content, std::fstream &file, std::string &line, std::map<std::string,std::map<std::string,std::string> >& _config);
		int									checkRoutes(int &isRoute, std::string &content, std::map<std::string,std::map<std::string,std::string> > _config);
		void 								printData();
		void								parseCustomErr();
		void								parseIndexFiles();
		void								checkMaxBodySize(void);
		void								addCustomErrorInVect(std::string errParam);
		void								addIndexFilesInVect(std::string indexParam);
		int									isEmpty(std::string conf);

		//random exceptions just in case
		class	CantOpenFileException : public std::exception{
		public:
			const char	*what() const throw(){ return "can't open file";};
		};
		class	EmptyFileException : public std::exception{
		public:
			const char	*what() const throw(){ return "empty configuration file";};
		};
		class	WrongDataException : public std::exception{
		public:
			const char	*what() const throw(){ return "wrong data";};
		};
		class	DuplicateServerException : public std::exception{
		public:
			const char	*what() const throw(){ return "duplicate server, try changing server_name or listen port";};
		};

		//get
		
		std::vector<std::map<std::string, std::map<std::string, std::string> > >&	getServers();
		std::vector<int>								getPorts();
		int									getPortsNbr();
		std::vector<std::string>&			getRoutes();
};

//it can't compile with this

//#include "../HTTP/Response.hpp"
//Response	*requestParse(std::string request, Server &server);
