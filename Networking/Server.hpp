/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmidon <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/04 09:54:40 by mmidon            #+#    #+#             */
/*   Updated: 2023/05/04 11:14:33 by mmidon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include <vector>
#include <map>
#include "ListeningSocket.hpp"
#include <fstream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include "../HTTP/Response.hpp"

class Server {
	private:
		char							_requestBuffer[30000];
		std::vector<ListeningSocket*>	_socket;
		int								_requestFd;
		void							_accepter(int server_fd);
		void							_handler(int client_fd);
		void							_responder(int client_fd, Response *responder);
		void 							_watchLoop();
		struct kevent					_evSet;
		struct kevent					_evList[32];
		struct sockaddr_storage			_addr;
		socklen_t						_socklen;
		int								_kq;



	public:
		Server(int domain, int service, int protocole, int *ports, int nbSocket);
		~Server(void);
		ListeningSocket	*getSocket(void) const;
		void			start(void);
};

#endif
