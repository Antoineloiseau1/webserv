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
		int				_type;

	public:
		enum		_status 
		{
			INIT,
			HEADER_PARSED,
			PARSING_PREBODY,
			READY_FOR_DATA,
			BODY_PARSED,
			RESPONSE,
			OVER
		};
		enum	_type { GET, POST_DATA, POST_FORM, DELETE };

		bool			readyForData;

		Client(int fd, int serverFd);
		~Client();

		void	parsePreBody(char *buf, int size);

/******************************GETTER*************************************/
		
		int				getType();
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
		void			setBodyBuf(char *buf);
		void			setBodyBufSize(int n);
		void			setPreBody(std::string pre_body);
		void			writeInFile(char *buf,int size);
		void			addOnBodyBuf(char *buf, int size);

};


#endif