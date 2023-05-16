#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>
#include "../HTTP/Request.hpp"
// # include <netinet/in.h>
// # include <iostream>
// # include <sys/socket.h>
// # include <stdio.h>
// # include <errno.h>
// # include <unistd.h>

class Client
{
	private:
		int			_fd;
		std::string	_reqBuf;
		int			_status;
		Request		*_request;

	public:
		enum		_status 
		{
			STANDBY,
			RECEIVED,
			RESPONSE,
			OVER
		};
		Client(int fd);
		~Client();

		int			getFd();
		int			getStatus();
		void		setStatus(int status);
		std::string	getReqBuf();
		void		addOnReqBuf(std::string buf);
		void		createRequest(std::string reqLine);
		Request		*getRequest();

};


#endif