#ifndef SERVER_HPP
# define SERVER_HPP

#include "ServerSocket.hpp"

class Server {
	private:
		char			_requestBuffer[30000];
		ServerSocket	*_socket;
		int				_requestFd;
		void			_accepter(void);
		void			_handler(void);
		void			_responder(std::string content);
		
	public:
		Server(int domain, int service, int protocole, int port, int backlog);
		~Server(void);
		ServerSocket	*getSocket(void) const;
		void			start(void);
};

#endif