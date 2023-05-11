#include "Response.hpp"
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

Response::Response(Request &request, Server &server) : _server(server), _request(request) {
	_response["version"] = _request.getVersion();
	_response["connexion"] = "Connexion: close\r\n\r\n";
}
Response::~Response() {}

GetResponse::GetResponse(Request request, Server& server) : Response(request, server) {


		std::string	file = request.getPath();
		if (file != "favicon.ico" && file != " " && !file.empty() && file != "" && file != "data/www/style.css")
			_response["body"] = openHtmlFile(file);
		else
			_response["body"] = "GetResponse: Not handled yet\r\n";
		_response["length"] = "Content-Length: ";
		_response["length"] += std::to_string(std::strlen(_response["body"].c_str()));
		_response["length"] += "\r\n";
		_response["type"] = "Content-Type: text/html\r\n"; //NEED TO PARSE
}

GetResponse::~GetResponse() {}

void	GetResponse::executor() {}

PostResponse::PostResponse(Request request, Server& server) : Response(request, server) {

		std::string	file = request.getPath();
		if (file != "favicon.ico" && file != " " && !file.empty() && file != "" && file != "data/www/style.css")
			_response["body"] = openHtmlFile(file);
		else
			_response["body"] = "GetResponse: Not handled yet\r\n";
		_response["length"] = "Content-Length: ";
		_response["length"] += std::to_string(std::strlen(_response["body"].c_str()));
		_response["length"] += "\r\n";
		_response["type"] = "Content-Type: text/html\r\n"; //NEED TO PARSE
	executor();
}

PostResponse::~PostResponse() {}

void	PostResponse::executor() {
    //   nom=a&prenom=a&email=a%40q&ville=a
	std::cout << "EXECUTE POST REQUEST\n";
	handleCgi();
}

DeleteResponse::DeleteResponse(Request request, Server& server) : Response(request, server) {}

DeleteResponse::~DeleteResponse() {}

void	DeleteResponse::executor() {
	std::cout << "EXECUTE Delete REQUEST\n";
}

std::string	Response::openHtmlFile(std::string f)
{
	std::ifstream file(f);
    if (!file.is_open())
	{
		_response["status"] = " 404 Not Found";
		return (openHtmlFile("data/www/error/404.html"));
	}
	else
	{
		_response["status"] = " 200 OK";
    	// read the contents of the file into a string variable
    	std::string content((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
		return content;
	}
}

std::map<std::string, std::string>	Response::getMap() { return _response; }

void	Response::handleCgi() {
	int pipefd[2];

	std::string arg1 = "form_handler.cgi";
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

std::string	Response::buildResponse(void) {
	return (_response["version"] + _response["status"] + _response["type"] + _response["length"] + _response["connexion"] + _response["body"]);
}

/********************************* BadRequestError Class ***************************************/
BadRequestError::BadRequestError(Request request, Server& server) : Response(request, server) {

	std::ifstream error("data/www/error/400.html");
    std::string content((std::istreambuf_iterator<char>(error)), (std::istreambuf_iterator<char>()));

	_response["version"] = _request.getVersion();
	_response["status"] = " 400 Bad Request\r\n";
	_response["body"] = content;
	_response["type"] = "Content-Type: text/html\r\n";
	_response["length"] = "Content-Length: ";
	_response["length"] += std::to_string(std::strlen(_response["body"].c_str()));
	_response["length"] += "\r\n";
	_response["connexion"] = "Connexion: close\r\n\r\n";
}

void	BadRequestError::executor(void) {}
BadRequestError::~BadRequestError(void) {}

/************************************************************************************************/