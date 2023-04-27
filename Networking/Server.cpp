#include "Server.hpp"
#include <fstream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#define NUSERS 10

// struct uc {
//     int uc_fd;
//     char *uc_addr;
// } users[NUSERS];

// void
// send_msg(int s, std::string message) {
//     char buf[256];
//     int len;

//     len = message.length();
//     send(s, buf, len, 0);
// }

// void recv_msg(int s) {
//     char buf[30000];
//     size_t bytes_read;

//     bytes_read = recv(s, buf, sizeof(buf), 0);
//     if ((int)bytes_read)
// 	{	
// 		buf[bytes_read] = 0;
//         std::cout << buf << std::endl;
// 	}
// }

// int conn_index(int fd) {
//     int index;
//     for (index = 0; index < NUSERS; index++)
//         if (users[index].uc_fd == fd)
//             return index;
//     return -1;
// }

// int conn_delete(int fd) {
//     int index;
//     if (fd < 1) 
// 		return -1;
//     if ((index = conn_index(fd)) == -1)
//         return -1;

//     users[index].uc_fd = 0;
//     users[index].uc_addr = NULL;

//     /* free(users[index].uc_addr); */
//     return close(fd);
// }

// int conn_add(int fd) {
//     int index;
//     if (fd < 1) 
// 		return -1;
//     if ((index = conn_index(0)) == -1)
//         return -1;
//     if (index == NUSERS) {
//         close(fd);
//         return -1;
//     }
//     users[index].uc_fd = fd; /* users file descriptor */
//     users[index].uc_addr = 0; /* user IP address */
//     return 0;
// }

void Server::_watchLoop(int kq) {
    struct kevent evSet;
    struct kevent evList[32];
    int nev;
    struct sockaddr_storage addr;
    socklen_t socklen = sizeof(addr);

    while(1) {
      nev = kevent(kq, NULL, 0, evList, 32, NULL);
        if (nev == -1) {
            perror("kevent() failed");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < nev; ++i) {
            if ((int)evList[i].ident == _socket[0]->getFd()) {
                // Handle incoming connection
                int conn_fd = accept(evList[i].ident, (struct sockaddr *)&addr, &socklen);
                if (conn_fd == -1) {
                    perror("accept() failed");
                    exit(EXIT_FAILURE);
                }

                // Add the connection socket to the kqueue
                EV_SET(&evSet, conn_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
                if (kevent(kq, &evSet, 1, NULL, 0, NULL) == -1) {
                    perror("kevent() failed");
                    exit(EXIT_FAILURE);
                }
            }
            else {
                // Handle incoming data
                char buf[1024];
                ssize_t n = recv(evList[i].ident, buf, sizeof(buf), 0);
                if (n == -1) {
                    perror("read() failed");
                    exit(EXIT_FAILURE);
                }
                else if (n == 0) {
                    // Connection closed by client
                    close(evList[i].ident);
                    EV_SET(&evSet, evList[i].ident, EVFILT_READ, EV_DELETE, 0, 0, NULL);
                    if (kevent(kq, &evSet, 1, NULL, 0, NULL) == -1) {
                    perror("kevent() failed");
                    exit(EXIT_FAILURE);
                    }
        		}
				 else {
					// Handle received data
					buf[n] = '\0';
					printf("Received data from %d: %s", (int)evList[i].ident, buf);
					// Add code to process the received data here
				}
    		}
		}
	}	
}


Server::Server(int domain, int service, int protocole, int *ports, int nbSocket) {
	for (int i = 0; i < nbSocket; i++)
		this->_socket.push_back(new ListeningSocket(domain, service, protocole, ports[i]));
}

void	Server::start(void) {
    // open the HTML file
    // std::ifstream file("./data/www/manon.html");
    // if (!file.is_open())
    // {
    //     std::cerr << "Error opening file" << std::endl;
    //     exit(EXIT_FAILURE);
    // }

    // // read the contents of the file into a string variable
    // std::string content((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
	int kq;
	struct kevent evSet;

	kq = kqueue();

	EV_SET(&evSet, _socket[0]->getFd(), EVFILT_READ, EV_ADD, 0, 0, NULL);
	if (kevent(kq, &evSet, 1, NULL, 0, NULL) == -1)
		std::cerr << "kevent: " << strerror(errno) << std::endl;

	_watchLoop(kq);
	// while(true) {
	// 	std::cout << "+++++++ Waiting for new connection ++++++++" << std::endl << std::endl;
	// 	_accepter();
	// 	_handler();
	// 	_responder(content);
	// 	close(this->_requestFd);
	// }
}

void	Server::_accepter(void) {
	struct sockaddr_in	address;
	socklen_t			addrlen;
	int					r;

	this->_requestFd = accept(this->_socket[0]->getFd(), reinterpret_cast<struct sockaddr *>(&address), &addrlen);
	if (this->_requestFd == -1) {
		delete this->_socket[0];
		std::cerr << "accept: " << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}

	//fcntl(this->_requestFd, F_SETFL, O_NONBLOCK);
	memset(this->_requestBuffer, 0, sizeof(this->_requestBuffer));
	r = read(this->_requestFd, this->_requestBuffer, 30000);
	std::cout << "Server::Accepter: ";
	if (r > 0) {
		this->_requestBuffer[r] = 0;
		std::cout << " ########### Received " << r << " bytes ###########\n" << std::endl;
	}
	else
		std::cout << " bytes = " << r << " nothing to read" << std::endl;
}

void	Server::_handler(void) {
	std::cout << this->_requestBuffer << std::endl;
}

void	Server::_responder(int fd) {
		std::string hello = "HTTP/1.1 200 OK\n";
		write(fd , hello.c_str() , hello.length());
}

ListeningSocket	*Server::getSocket(void) const { return this->_socket[0]; }

Server::~Server(void) { delete this->_socket[0]; }