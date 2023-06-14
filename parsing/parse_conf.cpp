/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_conf.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: anloisea <anloisea@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/25 10:15:25 by mmidon            #+#    #+#             */
/*   Updated: 2023/06/14 10:00:30 by mmidon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <cstdlib>
#include <cstring> //RAJOUTE POUR COMPILER SUR LINUX, VERIF SI STRNCMP OK

#include "parsing.hpp"

std::string trim(const std::string& str, const std::string& whitespace = " \t") //ft_trim but better
{
	const size_t strBegin = str.find_first_not_of(whitespace);
	if (strBegin == std::string::npos)
		return "";

	const size_t strEnd = str.find_last_not_of(whitespace);
	const size_t strRange = strEnd - strBegin + 1;

	return str.substr(strBegin, strRange);
}

void	data::makePorts(size_t serv)
{
	std::vector<std::string>	portTab;
	std::string					portStr = getServers()[serv]["default"]["listen"];
	std::string					tmp;
	for (size_t	i = 0; portStr[i]; i++)
	{
		if (portStr[i] == ' ' && !tmp.empty())
		{
			portTab.push_back(tmp);
			tmp.clear();
		}
		else
			tmp += portStr[i];
	}
	if (!tmp.empty())
		portTab.push_back(tmp);

	if (!portTab.size())
		portTab.push_back("80");
	if (portTab.size() > 1)
		throw(WrongDataException());
	int res = atoi(portTab[0].c_str());

	_ports.push_back(res);
	_portsNbr += 1;
}
std::vector<std::string>	data::getRoutes() {return _routes;}

std::vector<int>	data::getPorts() {return _ports;}

int		data::getPortsNbr() { return _portsNbr; }

std::vector<std::map<std::string, std::map<std::string, std::string> > >	data::getServers() { return _servers; }

void	data::setSettings() //put all the accepted settings (the keyword will also be the key for the value in the map)
{

	_onlyRouteSettings.push_back("limit_except");

	_routeSettings.push_back("listen");
	_routeSettings.push_back("autoindex");
	_routeSettings.push_back("cgi_extension");
	_routeSettings.push_back("listen");

	_possibleSettings.push_back("server_name");
	_possibleSettings.push_back("client_max_body_size");

	_possibleSettings.push_back("location");
	_possibleSettings.push_back("server");
	//a lot of things to push_back
}


std::string	data::whichSetting(std::string content) //change variable names later
{
	for (size_t i = 0; i != _routeSettings.size(); i++) //for both
	{
		if (!std::strncmp(content.c_str(), _routeSettings[i].c_str(), std::strlen(_routeSettings[i].c_str())) && (std::isspace(content[std::strlen(_routeSettings[i].c_str())]) || !content[std::strlen(_routeSettings[i].c_str())]))
			return _routeSettings[i];
	}

	for (size_t i = 0; i != _onlyRouteSettings.size() && isRoute; i++) //only for route
	{
		if (!std::strncmp(content.c_str(), _onlyRouteSettings[i].c_str(), std::strlen(_onlyRouteSettings[i].c_str())) && (std::isspace(content[std::strlen(_onlyRouteSettings[i].c_str())]) || !content[std::strlen(_onlyRouteSettings[i].c_str())]))
			return _onlyRouteSettings[i];
	}

	for (size_t i = 0; i != _possibleSettings.size() && !isRoute; i++) //not in route
	{
		if (!std::strncmp(content.c_str(), _possibleSettings[i].c_str(), std::strlen(_possibleSettings[i].c_str())) && (std::isspace(content[std::strlen(_possibleSettings[i].c_str())]) || !content[std::strlen(_possibleSettings[i].c_str())]))
			return _possibleSettings[i];
	}
	return "";
}

bool	isRooted(std::string const newRoute, std::vector<std::string>& _routes)
{
	for (std::vector<std::string>::iterator it = _routes.begin(); it != _routes.end(); it++)
	{
		if (*it == newRoute || !std::strncmp(newRoute.c_str(), (*it).c_str(), newRoute.length())|| !std::strncmp(newRoute.c_str(), (*it).c_str(), (*it).length()))
		{
			std::cerr << "Duplicated route" << std::endl;
			return true;
		}
	}
	return false;
}

void	data::newRouteSetup(std::string &content, std::fstream &file, std::string &line, std::map<std::string,std::map<std::string,std::string> > _config)
{
	isRoute++; //entering a route
	getline(file, content);
	content = trim(content);
	if (content != "{" || line.empty())
	{
		_config.erase(_config.begin(), _config.end());
		throw(WrongDataException());
	}
	fill(file, line); //recursive
}

int	data::checkRoutes(int &isRoute, std::string &content, std::map<std::string,std::map<std::string,std::string> > _config)
{
	if (content == "{") //parsing route syntax
	{
		if (isRoute == 1)
		{
			_config.erase(_config.begin(), _config.end());
			throw (WrongDataException());
		}
	}
	else if (content == "}" && isRoute) //end of route
	{
		isRoute--;// exiting the route
		return 1;
	}
	return 0;
}

void data::fill(std::fstream &file, std::string route) //at first call:  route="default"
{
	std::size_t	 pos = 0;
	std::string content;
	std::string setting;
	static std::vector<std::string>	server_routes;
	static std::map<std::string, std::map<std::string, std::string> >	_config;

	if (isRooted(route, server_routes))
	{
		_config.erase(_config.begin(), _config.end());
		throw (WrongDataException());
	}
	server_routes.push_back(route);
	_routes.push_back(route);

	while (getline(file, content) && isRoute >= 0) //now it accept random empty lines
	{
		if (content.empty())
			continue ;
		content = trim(content);

		if (content == "server")
		{
			if (!_config.size() && !_config["default"].size())
				continue;
			else
			{
				_servers.push_back(_config);
				server_routes.clear();
				_config.erase(_config.begin(), _config.end());
				continue;
			}
		}

		if (checkRoutes(isRoute, content, _config))
			return ;
		setting = whichSetting(content); //find which setting is at the beginning of the line (ignore spaces and tabs)

		if (setting.empty()) //error handling
		{
			_config.erase(_config.begin(), _config.end());
			throw (WrongDataException());
		}

		pos = content.find(setting) + std::strlen(setting.c_str());
		std::string line = content.substr(pos, content.find(";", pos) - pos); //find the content at the end of the line
		line = trim(line); //spaces handling

		if (setting == "location") //route handling
		{
			newRouteSetup(content, file, line, _config);
			continue;
		}

		if (line.empty()) //error handling
		{
			_config.erase(_config.begin(), _config.end());
			throw (WrongDataException());
		}
		_config[route].insert(std::make_pair(setting, line)); //put it in the config variable
	}

	_servers.push_back(_config);
	_config.erase(_config.begin(), _config.end());
	if (isRoute != 0) //if the int isnt 0 then it's a route parsing error
		{
			_config.erase(_config.begin(), _config.end());
			throw (WrongDataException());
		}
}

void data::printData()
{
	for (size_t i = 0; i != _servers.size(); i++)
	{
		std::cerr << "SERVER : " << i << std::endl << std::endl;
		for (std::map<std::string, std::map<std::string, std::string> >::iterator route = _servers[i].begin(); route != _servers[i].end(); route++)
		{
			std::cerr << "\nROUTE : " << route->first << std::endl << std::endl;
			for (std::map<std::string, std::string>::iterator it = _servers[i][route->first].begin(); it != _servers[i][route->first].end(); *it++)
				std::cerr << it->first << " | " << it->second << std::endl;
		}
	}
}

data::data(std::string conf)
{
	isRoute = 0;
	_portsNbr = 0;
	try
	{
		std::fstream file(conf); //can't fine if i need ofstream or ifstream so fstream
		if (!file.is_open())
			throw CantOpenFileException();

		setSettings();
		data::fill(file, "default");
	}
	catch (std::exception &e)
	{
		_servers.erase(_servers.begin(), _servers.end());
		std::cerr << "Exception caught while parsing config file: " << e.what() << std::endl;
		exit (1);
	}
	try
	{
		for (size_t i = 0; i != _servers.size(); i++)
			makePorts(i);
	}
	catch (std::exception &e)
	{
		std::cerr << "Exception caught while setting ports: " << e.what() << std::endl;
		exit (2);
	}
}

data::~data()
{
	return;
}
