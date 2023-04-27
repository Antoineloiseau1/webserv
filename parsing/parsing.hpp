/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmidon <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/27 10:31:47 by mmidon            #+#    #+#             */
/*   Updated: 2023/04/27 11:39:30 by mmidon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include <iostream>
#include <exception>

class data
{
	private:
		std::map<std::string, std::string> _config;
		std::string _name;
	public:
		data(std::string conf);
		~data();
		
		//random exception just in case
		class CantOpenFileException : public std::exception{
		public:
			 const char *what() const throw(){ return "can't open file";};
		};
		class WrongDataException : public std::exception{
		public:
			 const char *what() const throw(){ return "wrong data";};
		};
};
