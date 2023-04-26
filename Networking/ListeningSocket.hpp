#ifndef ListeningSocket_HPP
# define ListeningSocket_HPP

# include "Socket.hpp"

class ListeningSocket: public Socket {

	public:

		ListeningSocket(int domain, int service, int protocole, int port);
		~ListeningSocket(void);

		void				establishConnection(void);
		int					getFd(void) const;
		socklen_t			getSockLen(void) const;
		struct sockaddr_in	getAddress(void) const;

};

#endif