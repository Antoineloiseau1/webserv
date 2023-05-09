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

std::string	cgiHandling(std::string f)
{
	std::ifstream file(f);
    if (!file.is_open())
    {
		return (cgiHandling("data/www/error.html"));
    }

    // read the contents of the file into a string variable
    std::string content((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
	return content;
}

std::string	cutRequest(std::istringstream &iss, std::string limiter) //cut it in little parts easy to understand and to interpret
{
	std::string result;

	std::getline(iss, result);

	result = result.substr(result.find("/") + 1 , result.find(limiter) - std::strlen(limiter.c_str()) - 2);

	return result;
}

std::map<std::string, std::string>	requestParse(std::string request)
{
	std::map<std::string, std::string> response;
	std::istringstream iss(request);
	std::string file = cutRequest(iss, "HTTP");

	response["status"] = "HTTP/1.1 200 OK\r\n";
	response["type"] = "Content-Type: text/plain\r\n";
	response["connexion"] = "Connexion: close\r\n\r\n";

	if (file != "favicon.ico" && file != " " && !file.empty() && file != "" && file != "data/www/style.css") ///terrible conditions bit i'll change it later
	{
		response["body"] = cgiHandling(file);
		response["type"] = "Content-Type: text/html\r\n";
	}
	else
		response["body"] = "Hello, World!";
	response["length"] = "Content-Length: ";
	response["length"] += std::to_string(std::strlen(response["body"].c_str()));
	response["length"] += "\r";

	return response;
}
