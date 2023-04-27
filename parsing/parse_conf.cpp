/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_conf.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmidon <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/25 10:15:25 by mmidon            #+#    #+#             */
/*   Updated: 2023/04/27 11:45:16 by mmidon           ###   ########.fr       */
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

data::data(std::string conf)
{
	std::size_t	i = 0;

	std::fstream file(conf); //can't fine which of ofstream and ifstream we need so fstream
	if (!file.is_open())
		throw CantOpenFileException();

	std::string content((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));

	i = content.find("listen") + std::strlen("listen");   //"listen" is tmp, it should be a loop on a list of "authorized" parameters
	std::string line = content.substr(i, content.find(";", i) - i);
	line = trim(line);
	_config.insert(std::make_pair("listen", line));
	std::cout << "line:" << _config["listen"]  << "$" << std::endl;



	exit(666);
}

data::~data()
{
	std::cout << "Parsing Finished" << std::endl;
	return;
}
