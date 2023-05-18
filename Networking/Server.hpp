/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antoine <antoine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/04 09:54:40 by mmidon            #+#    #+#             */
/*   Updated: 2023/05/18 11:12:17 by antoine          ###   ########.fr       */
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
#include <sys/select.h>
#include <sys/time.h>
#include "../HTTP/Response.hpp"
#include <stdlib.h>

class Response;

class Server {
	private:
		char							_requestBuffer[30000];
		std::vector<ListeningSocket*>	_socket;
		int								_requestFd;
		void							_accepter(int server_fd, ListeningSocket *socket);
		void							_refuse(int server_fd);
		void							_handler(Client *client);
		void							_responder(Client *client);
		void 							_watchLoop();
		fd_set							_readSet;
		fd_set							_writeSet;
		int								_fdMax;
		struct sockaddr_storage			_addr;
		socklen_t						_socklen;
		char							**_envp;



	public:
		Server(int domain, int service, int protocole, int *ports, int nbSocket, char **envp);
		~Server(void);
		ListeningSocket	*getSocket(void) const;
		ListeningSocket	*getSocket(int fd);
		void			start(void);
		int				getRequestFd() const;
		char			**getEnvp() const;
		int				getOpenFd();

		void	change_events(std::vector<struct kevent>& change_list, uintptr_t ident, int16_t filter,
       				uint16_t flags, uint32_t fflags, intptr_t data, void *udata);
};

#endif
