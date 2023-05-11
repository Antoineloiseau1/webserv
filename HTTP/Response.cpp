#include "Response.hpp"
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

void	Response::handleCgi() {
	int pipefd[2];

	std::string arg1 = "data/CGI/form_handler.cgi";
	const char* cmd1_cstr = arg1.c_str();
	std::string arg2 = _request.getBody();
	const char* cmd2_cstr = arg2.c_str();
	char* const args[] = { const_cast<char*>(cmd1_cstr), const_cast<char*>(cmd2_cstr), NULL };

	if (pipe(pipefd) == -1) {
		std::cerr << "Erreur lors de la création de la pipe\n";
		close(_server.getSocket()->getFd());
		return ;
	}
	std::cout << "PIPE a fermer = " <<  pipefd[1] << std::endl;
	close(pipefd[0]);

	pid_t pid = fork();
	if (pid == -1) {
		std::cerr << "Erreur lors du fork\n";
		close(_server.getSocket()->getFd());
		close(pipefd[0]);
		close(pipefd[1]);
		return ;	
	}

	if (pid == 0) { // Processus enfant (script CGI)
		pipefd[1] = _server.getRequestFd();
		dup2(pipefd[1], STDOUT_FILENO);
		if (execve("data/CGI/form_handler.cgi", args, _server.getEnvp()) == -1)
		{
			std::cerr << "error: " << strerror(errno) << std::endl;
		}
	}
	close(pipefd[1]);
	close(pipefd[0]);
	
}

Response::Response(Request &request, Server &server) : _server(server), _request(request)
{
	try {
	std::string	file = request.getPath();
	_response["status"] = "200 OK\r\n";
	if (file != "favicon.ico" && file != " " && !file.empty() && file != "" && file != "data/www/style.css")
		_response["body"] = openHtmlFile(request.getPath());
	else
		_response["body"] = "Hello World";
	_response["length"] = "Content-Length: ";
	_response["length"] += std::to_string(std::strlen(_response["body"].c_str()));
	_response["length"] += "\r";
	_response["type"] = "Content-Type: text/html\r\n"; //NEED TO PARSE
	_response["version"] = request.getVersion();
	_response["connexion"] = "Connexion: close\r\n\r\n";
	}
	catch( std::out_of_range &e) {
		std::cout << "ERROR: " << e.what() << std::endl;
	}
	if (request.getPath().find(".cgi") != std::string::npos) {
		std::cerr << "Entering handle cgi... \n";
		handleCgi();
	}
}

Response::~Response() {}

std::string	Response::openHtmlFile(std::string f)
{
	std::ifstream file(f);
	std::cout << "-Opening HTML file : " << f << "\n";
    if (!file.is_open())
    {
		std::cout << "Trying to open error page\n";
		return (openHtmlFile("data/www/error.html"));
    }

    // read the contents of the file into a string variable
    std::string content((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
	if (content.empty())
	{
		std::cout << "Trying to open error page\n";
		return (openHtmlFile("data/www/error.html"));
    }
	return content;
}

std::map<std::string, std::string>	Response::getMap() { return _response; }

GetResponse::GetResponse(Request request, Server& server) : Response(request, server) {}

GetResponse::~GetResponse() {}

void	GetResponse::executor() {}

PostResponse::PostResponse(Request request, Server& server) : Response(request, server) {
	executor();
}

PostResponse::~PostResponse() {}

void	PostResponse::executor() {
    //   nom=a&prenom=a&email=a%40q&ville=a
	std::cout << "EXECUTE POST REQUEST\n";

	// handleCgi();
}

DeleteResponse::DeleteResponse(Request request, Server& server) : Response(request, server) {}

DeleteResponse::~DeleteResponse() {}

void	DeleteResponse::executor() {
	std::cout << "EXECUTE Delete REQUEST\n";
}