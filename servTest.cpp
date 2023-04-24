/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   servTest.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmidon <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/20 14:32:43 by mmidon            #+#    #+#             */
/*   Updated: 2023/04/24 10:24:47 by mmidon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include <sys/uio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <fstream>

#include "webserv.hpp"

int main(int ac, char **av, char **envp)
{
	//init socket
	int serverfd = socket(AF_INET, SOCK_STREAM, 0);

	if (serverfd < 0)
	{
		std::cout << "Error" << std::endl;
		return 666;
	}

	//init struct
	struct sockaddr_in address;

	//set struct
	address.sin_family = AF_INET;
	address.sin_port = htons(8080);
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	socklen_t addresslen = sizeof(address);

	if (bind(serverfd, (struct sockaddr*)&address, sizeof(address)) < 0)
	{
		perror("In bind ");
		return 666;
	}

	//wait

	if (listen(serverfd, 10) < 0)
	{
		perror("Error : ");
		return 2;
	}

	int new_socket;
	while (1)
	{
		std::cout << "\n+++++++ Waiting for new connection ++++++++\n\n";
		if ((new_socket = accept(serverfd, (struct sockaddr *)&address, (socklen_t*)&addresslen)) < 0 )
		{
			perror("In accept : ");
			exit(666);
		}

		//read
		std::string request;
		char buffer[1024] = {0}; //1024 = random define BUFFER_SIZE
		int rd = read(new_socket, buffer, 1024);
		request.append(buffer, rd);
		std::cout << "\n\n request --> " << request << std::endl;
		std::cout <<"find " << request.find("/cplusplus.cgi")<< std::endl;
		if (request.find("/cplusplus.cgi") != std::string::npos)
		{
		// Forker le processus pour exécuter le script CGI
		int pipefd[2];
		if (pipe(pipefd) == -1) {
			std::cerr << "Erreur lors de la création de la pipe\n";
			close(serverfd);
			continue;
		}

		pid_t pid = fork();
		if (pid == -1) {
			std::cerr << "Erreur lors du fork\n";
			close(serverfd);
			close(pipefd[0]);
			close(pipefd[1]);
			continue;
		}

		if (pid == 0) { // Processus enfant (script CGI)
			pipefd[1] = new_socket;
			dup2(pipefd[1], STDOUT_FILENO);
			std::cerr << pipefd[1] << "zui " <<std::endl;
			execve("cplusplus.cgi",NULL, envp);
			std::cout << "\n\n\n Executed... but wait \n\n";
		}
		std::cerr << pipefd[1] << "zui2 " <<std::endl;
		close(pipefd[1]);
		close(pipefd[0]);
		}
		else
		{
			//send random thing to the server
			char *hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 11\n\nthe worldo!";
			std::cout <<hello << buffer << std::endl;
			write(new_socket, hello, strlen(hello));
		}
		//close
		close(new_socket);
	}
	close(serverfd);
	return 0;
}
