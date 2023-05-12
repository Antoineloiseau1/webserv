/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_request.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: anloisea <anloisea@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/04 10:37:45 by mmidon            #+#    #+#             */
/*   Updated: 2023/05/12 11:37:22 by anloisea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <map>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include "../HTTP/Request.hpp"
#include "../HTTP/Response.hpp"
#include "../Networking/Server.hpp"
#include <exception>


std::string	cutRequest(std::istringstream &iss, std::string limiter) //cut it in little parts easy to understand and to interpret
{
	std::string result;

	std::getline(iss, result);

	result = result.substr(result.find("/") + 1 , result.find(limiter) - std::strlen(limiter.c_str()) - 2);

	return result;
}

