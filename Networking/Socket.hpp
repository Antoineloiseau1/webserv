#ifndef SOCKET_HPP
# define SOCKET_HPP

# include <netinet/in.h>
# include <iostream>
# include <sys/socket.h>
# include <stdio.h>
# include <errno.h>
# include <unistd.h>
# include <stdlib.h>
# include <cstring>


class Socket{

	protected:

		struct sockaddr_in	_address;
		socklen_t			_addrlen;
		int					_fd;

	public:

		Socket(int domain, int service, int protocole, int port);
		virtual	~Socket(void);

		virtual int					getFd(void) const = 0;
		virtual struct sockaddr_in	getAddress(void) const = 0;	

};

#endif
