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
		ListeningSocket	*_server;
		std::string		_reqBuf;
		int				_status;
		Request			*_request;

	public:
		enum		_status 
		{
			STANDBY,
			RECEIVED,
			RESPONSE,
			OVER
		};
		Client(int fd, ListeningSocket *server);
		~Client();

		int				getFd();
		int				getStatus();
		void			setStatus(int status);
		std::string		getReqBuf();
		void			addOnReqBuf(std::string buf);
		void			createRequest(std::string reqLine);
		Request			*getRequest();
		ListeningSocket	*getServer();

};


#endif