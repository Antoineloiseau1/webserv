#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>
#include "../HTTP/Request.hpp"
#include <fstream>

#define BUFFER_SIZE 424242


class Client
{
	private:
		int				_fd;
		int				_serverFd;
		int				_status;
		Request			*_request;
		std::string		_tmpPictFile;
		std::ofstream 	_file;
		std::string		_preBody;
		std::string		_formBody;
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
		enum	_type { GET_DELETE, POST_DATA, POST_FORM };

		bool			readyForData;
		long			bytes;

		Client(int fd, int serverFd, std::string tmp_file);
		~Client();

		int				parsePreBody(char *buf, int size);

/******************************GETTER*************************************/
		
		int				getType();
		int				getFd();
		int				getStatus();
		Request			*getRequest();
		int				getServerFd();
		std::ofstream	&getFile();
		std::string		getFormBody();
		std::string		getTmpPictFile();


/******************************SETTER*************************************/
		void			createRequest(char *reqLine);
		void			setStatus(int status);
		void			writeInFile(char *buf,int size);
		void			setFormBody(std::string buf);

};


#endif
