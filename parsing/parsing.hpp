/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmidon <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/27 10:31:47 by mmidon            #+#    #+#             */
/*   Updated: 2023/05/23 08:50:56 by mmidon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include <vector>
#include <iostream>
#include <exception>
#include "../HTTP/Response.hpp"

class data
{
	private:
		std::map<std::string, std::string>	_config;
		std::string							_name;
		std::vector<std::string>			_possibleSettings;
		void								fill(std::string conf);
		int*								_ports;
		int									_portsNbr;
	public:
		data(std::string conf);
		~data();

		void								makePorts();
		void								setSettings();
		std::string							whichSetting(std::string line);

		//random exceptions just in case
		class	CantOpenFileException : public std::exception{
		public:
			const char	*what() const throw(){ return "can't open file";};
		};
		class	WrongDataException : public std::exception{
		public:
			const char	*what() const throw(){ return "wrong data";};
		};

		//get
		
		std::map<std::string, std::string>	getData();
		int*								getPorts();
		int									getPortsNbr();
};

Response	*requestParse(std::string request, Server &server);
