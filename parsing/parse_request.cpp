/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_request.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmidon <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/04 10:37:45 by mmidon            #+#    #+#             */
/*   Updated: 2023/05/09 10:26:49 by mmidon           ###   ########.fr       */
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

Response	*requestParse(std::string request, Server &server)
{
	//request == par ex, GET /data/www/about.html HTTP/1.1
	Response	*ret = 0;
	Request	clientRequest(request);
	std::string	type[] = { "GET", "POST", "DELETE" };
	int a = 0;

	for (int i = 0; i < 3 ; i++) {
		if (clientRequest.getType() == type[i])
			a = i + 1;
	}
	switch (a)
	{
		case 1:
			ret = new GetResponse( request, server );
			break;

		case 2:
			ret = new PostResponse( request, server );
			break;

		case 3:
			ret = new DeleteResponse( request, server );
			break;
		
		default:
			std::cout << "Bad Request" << std::endl;
	}
	
	return ret;
}
