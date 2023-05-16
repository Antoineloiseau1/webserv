#ifndef ListeningSocket_HPP
# define ListeningSocket_HPP

# include "Socket.hpp"
# include "Client.hpp"
# include <vector>

class ListeningSocket: public Socket {

	private:
		std::vector<Client*>	_clients;
	
	public:

		ListeningSocket(int domain, int service, int protocole, int port);
		~ListeningSocket(void);

		void	setClient(Client *newClient);
		int		getOpenFd();
		Client	*getClient(int fd);
		void	deleteClient(int fd);

		void				establishConnection(void);
		int					getFd(void) const;
		socklen_t			getSockLen(void) const;
		struct sockaddr_in	getAddress(void) const;

};

#endif