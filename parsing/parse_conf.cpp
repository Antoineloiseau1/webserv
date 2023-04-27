/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_conf.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmidon <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/25 10:15:25 by mmidon            #+#    #+#             */
/*   Updated: 2023/04/27 16:41:20 by mmidon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fstream>
#include <iostream>
#include <map>

#include "parsing.hpp"

std::string trim(const std::string& str, const std::string& whitespace = " \t")
{
	const size_t strBegin = str.find_first_not_of(whitespace);
	if (strBegin == std::string::npos)
		return "";

	const size_t strEnd = str.find_last_not_of(whitespace);
	const size_t strRange = strEnd - strBegin + 1;

	return str.substr(strBegin, strRange);
}

void data::setSettings()
{
	_possibleSettings.push_back("listen");
	_possibleSettings.push_back("");

	//a lot of things to push_back
}


std::string data::whichSetting(std::string content)
{
	for (int i = 0; !_possibleSettings[i].size(); i++)
	{
		if (!std::strncmp(content.c_str(), _possibleSettings[i].c_str(), std::strlen(_possibleSettings[i].c_str())))
			return _possibleSettings[i];
	}
	return "";
}


data::data(std::string conf)
{
	std::size_t	 pos = 0;
	std::string content;
	std::string setting;

	std::fstream file(conf); //can't fine which of ofstream and ifstream we need so fstream
	if (!file.is_open())
		throw CantOpenFileException();

	setSettings();
	getline(file, content);

	while (!content.empty())
	{
		getline(file, content);
		setting = whichSetting(content);
		if (setting.empty())
			throw (WrongDataException());
		pos = content.find(setting) + std::strlen(setting.c_str());
		std::string line = content.substr(pos, content.find(";", pos) - pos);
		line = trim(line);
		_config.insert(std::make_pair(setting, line));
		std::cout << "line:" << _config[setting]  << "$" << std::endl;
	}



	exit(666);
}

data::~data()
{
	std::cout << "Parsing Finished" << std::endl;
	return;
}
