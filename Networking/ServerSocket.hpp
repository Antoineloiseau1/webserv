#ifndef SERVERSOCKET_HPP
# define SERVERSOCKET_HPP

# include "Socket.hpp"

class ServerSocket: public Socket {

	public:

		ServerSocket(int domain, int service, int protocole, int port, int backlog);
		~ServerSocket(void);

		void				establishConnection(void);
		int					acceptConnection();
		int					getFd(void) const;
		socklen_t			getSockLen(void) const;
		struct sockaddr_in	getAddress(void) const;

};

#endif