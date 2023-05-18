#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>
#include "../HTTP/Request.hpp"

#define BUFFER_SIZE 424242

class ListeningSocket;


class Client
{
	private:
		int				_fd;
		int				_serverFd;
		int				_status;
		Request			*_request;
		char			*_reqBuf;

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

/******************************GETTER*************************************/
		int				getFd();
		int				getStatus();
		std::string		getReqBuf();
		Request			*getRequest();
		int				getServerFd();

/******************************SETTER*************************************/
		void			createRequest(char *reqLine);
		void			setStatus(int status);
		void			setReqBuf(char *buf); //sera surement modifier pour concatener

};


#endif