#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>
#include "../HTTP/Request.hpp"
#include <fstream>

#define BUFFER_SIZE 424242

class ListeningSocket;


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
			INIT,
			HEADER_PARSED,
			PRE_BODY_PARSED,
			BODY_PARSED,
			RESPONSE,
			OVER
		};
		bool			readyForData;
		Client(int fd, int serverFd);
		~Client();

/******************************GETTER*************************************/
		int				getFd();
		int				getStatus();
		char 			*getBodyBuf();
		Request			*getRequest();
		int				getServerFd();
		int				getBodyBufSize();
		std::ofstream	&getFile();
		int				getPreBodySize();
		


/******************************SETTER*************************************/
		void			createRequest(char *reqLine);
		void			setStatus(int status);
		void			setBodyBuf(char *buf); //sera surement modifier pour concatener
		void			setBodyBufSize(int n);
		// void			initFile();
		void			setPreBody();
		void			writeInFile(char *buf,int size);

};


#endif