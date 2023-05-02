/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_conf.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmidon <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/25 10:15:25 by mmidon            #+#    #+#             */
/*   Updated: 2023/05/02 15:06:58 by mmidon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fstream>
#include <iostream>
#include <map>

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

void data::setSettings() //put all the accepted settings (the keyword will also be the key for the value in the map)
{
	_possibleSettings.push_back("listen");
	_possibleSettings.push_back("server_name");
	_possibleSettings.push_back("client_max_body_size");

	//a lot of things to push_back (maybe put it in a file if the subject is ok with that)
}


std::string data::whichSetting(std::string content)
{
	for (size_t i = 0; i != _possibleSettings.size(); i++)
	{
		if (!std::strncmp(content.c_str(), _possibleSettings[i].c_str(), std::strlen(_possibleSettings[i].c_str())))
			return _possibleSettings[i];
	}
	return "";
}

void data::fill(std::string conf)
{
	std::size_t	 pos = 0;
	std::string content;
	std::string setting;

	std::fstream file(conf); //can't fine if i need ofstream or ifstream so fstream
	if (!file.is_open())
		throw CantOpenFileException(); //exception may not be the best choice

	setSettings();

	while (getline(file, content)) //so it doesnt accept random empty lines
	{
		content = trim(content);
		setting = whichSetting(content); //find which setting is at the beginning of the line (ignore spaces and tabs)

		if (setting.empty()) //error handling
		{
			_config.erase(_config.begin(), _config.end());
			throw (WrongDataException());
		}

		pos = content.find(setting) + std::strlen(setting.c_str());
		std::string line = content.substr(pos, content.find(";", pos) - pos); //find the content at the end of the line
		line = trim(line); //spaces handling


		_config.insert(std::make_pair(setting, line)); //put it in the config variable
	}
}

std::map<std::string, std::string> data::getData()
{
	return _config;
}

data::data(std::string conf) //search for each line in the conf an equivalent in the "possible settings"
{
	try
	{
		data::fill(conf);
	}
	catch (std::exception &e)
	{
		std::cout << "Exception caught : " << e.what() << std::endl;
	}
}

data::~data()
{
	return;
}
