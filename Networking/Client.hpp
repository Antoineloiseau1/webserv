#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>
#include "../HTTP/Request.hpp"
// #include "ListeningSocket.hpp"

class ListeningSocket;


class Client
{
	private:
		int				_fd;
		int				_serverFd;
		// ListeningSocket	**_server;
		std::string		_reqBuf;
		int				_status;
		Request			*_request;

	public:
		enum		_status 
		{
			INIT,
			HEADER_PARSED,
			BODY_PARSED,
			RESPONSE,
			OVER
		};
		Client(int fd, int serverFd);
		~Client();

		int				getFd();
		int				getStatus();
		void			setStatus(int status);
		std::string		getReqBuf();
		void			addOnReqBuf(std::string buf);
		void			createRequest(std::string reqLine);
		Request			*getRequest();
		int				getServerFd();

};


#endif