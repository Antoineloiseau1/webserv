#ifndef REQUEST_HPP
# define REQUEST_HPP

#include <vector>
#include "ListeningSocket.hpp"
#include <fstream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <string>

class Request
{
	private:
		std::string	_request;
		enum _eType {
        GET,
		POST,
		DELETE
    	};
		int	type;
    
		std::string	_type;
	public:
		Request(/* args */);
		~Request();
};

Request::Request(/* args */)
{
}

Request::~Request()
{
}


#endif