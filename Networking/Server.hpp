#ifndef SERVER_HPP
# define SERVER_HPP

#include <vector>
#include "ListeningSocket.hpp"

class Server {
	private:
		char							_requestBuffer[30000];
		std::vector<ListeningSocket*>	_socket;
		int								_requestFd;
		void							_accepter(void);
		void							_handler(void);
		void							_responder(std::string content);
		
	public:
		Server(int domain, int service, int protocole, int *ports, int nbSocket);
		~Server(void);
		ListeningSocket	*getSocket(void) const;
		void			start(void);
};

#endif