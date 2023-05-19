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
		int				_fd;
		int				_serverFd;
		int				_status;
		Request			*_request;
		char			_bodyBuf[BUFFER_SIZE];
		int				_bodyBufSize;
		std::ofstream 	_file;
		std::string		_preBody;

	public:
		enum		_status 
		{
			STANDBY,
			RECEIVED,
			RESPONSE,
			OVER
		};
		bool			readyForData;
		Client(int fd, int serverFd);
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