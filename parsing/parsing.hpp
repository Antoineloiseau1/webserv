/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmidon <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/27 10:31:47 by mmidon            #+#    #+#             */
/*   Updated: 2023/06/05 08:19:46 by mmidon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include <vector>
#include <iostream>
#include <exception>

class data
{
	private:
		std::map<std::string, std::map<std::string, std::string> >	_config;
		std::string							_name;
		std::vector<std::string>			_possibleSettings;
		std::vector<std::string>			_routes;
		void								fill(std::fstream &file, std::string route);
		int*								_ports;
		int									_portsNbr;
	public:
		data(std::string conf);
		~data();

		void								makePorts();
		void								setSettings();
		std::string							whichSetting(std::string line);

		int									isRoute;

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
		
		std::map<std::string, std::map<std::string, std::string> > getData();
		int*								getPorts();
		int									getPortsNbr();
		std::vector<std::string>			getRoutes();
};

//it can't compile with this

//#include "../HTTP/Response.hpp"
//Response	*requestParse(std::string request, Server &server);
