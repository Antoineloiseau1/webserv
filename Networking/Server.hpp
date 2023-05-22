/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elpolpa <elpolpa@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/04 09:54:40 by mmidon            #+#    #+#             */
/*   Updated: 2023/05/22 21:56:42 by elpolpa          ###   ########.fr       */
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
#include <cstring>
#include <string.h>
#include <iostream>

// # define BUFFER_SIZE 3000000

class Response;

class Server {
	private:
		char							_requestBuffer[BUFFER_SIZE];
		std::vector<ListeningSocket*>	_socket;
		int								_requestFd;
		void							_accepter(int server_fd, ListeningSocket *socket);
		void							_refuse(int server_fd);
		void							_handler(Client *client);
		void							_responder(Client *client);
		int								_getFdMax(void);
		void 							_watchLoop();
		fd_set							_readSet;
		fd_set							_writeSet;
		fd_set							_errorSet;
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
		static void		exit(int sig);
		void			disconnectClient(Client *client);
		void			setToWrite(Client *client);

};

#endif
