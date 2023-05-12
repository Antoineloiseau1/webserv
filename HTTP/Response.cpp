#include "Response.hpp"
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

Response::Response(Request &request, Server &server) : _server(server), _request(request) {
	std::string	type[] = { "GET", "POST", "DELETE", "HEAD", "OPTIONS", "PUT", "TRACE", "CONNECT" };
	enum	mtype { GET, POST, DELETE, OTHER };
	int a = 0;

	for (int i = 0; i < 8 ; i++) {
		if (request.getType() == type[i])
		{
			if (i > 2)
			{
				a = 3;
				break;
			}
			a = i;
		}
	}

	switch (a)
	{
		case GET:
			GetResponse();
			break;
		case POST:
			PostResponse();
			break;
		case DELETE:
			DeleteResponse();
			break;
		case OTHER:
			NotImplemented();
			break;
		default:
			BadRequestError();
	}
	_response["version"] = _request.getVersion();
	_response["connexion"] = "Connexion: close\r\n\r\n";
}

void	Response::GetResponse(void) {


		std::string	file = _request.getPath();
		if (file == "")
			_response["body"] = openHtmlFile("data/www/manon.html");
		else if (file == "favicon.ico" || file == "style.css" || file.empty())
		{
			_response["status"] = " 204 No Content\r\n";
			_response["body"] = "";
		}
		else
			_response["body"] = openHtmlFile(file);
		_response["length"] = "Content-Length: ";
		_response["length"] += std::to_string(std::strlen(_response["body"].c_str()));
		_response["length"] += "\r\n";
		_response["type"] = "Content-Type: text/html\r\n"; //NEED TO PARSE
}


/*
REQUEST BODY IN CASE OF UPLOAD EXPECTED:
------WebKitFormBoundary{boundary}
Content-Disposition: form-data; name="image"; filename="{filename}"
Content-Type: {mime-type}

{file-data}

------WebKitFormBoundary{boundary}--*/
void	Response::PostResponse(void) {
	/*---------------PARSING POST BODY FOR UPLOADING PICTURE REQUEST ONLY*/
		// std::map<std::string, std::string>	img_data;

		// std::string content_type = _request.getHeaders()["Content-Type"];
		// int loc_delim = content_type.find("boundary=") + 9;
		// std::string	boundary = content_type.substr(loc_delim, content_type.size() - loc_delim);

		// std::string line;
		// std::istringstream iss(_request.getBody());

		// std::getline(iss, line);
		// while(!line.empty() && line != "--" + boundary && line != "\r")
		// {
		// 	int delim = line.find_first_of(':');
		// 	img_data[line.substr(0, delim)] = line.substr(delim + 2, line.size() - (delim + 2));
		// 	getline(iss, line);
		// }
		// getline(iss, line);
		// while(line != "--" + boundary + "--" && line != "\r")
		// {
		// 	img_data["img_data"] += line;
		// 	getline(iss, line);
		// }
		// int loc = img_data["Content-Disposition"].find("filename=") + 9;
		// std::string file_name = img_data["Content-Disposition"].substr(loc, img_data["Content-Disposition"].size() - loc); 
    	// if (file_name.empty()) {
		// 	std::cerr << "Failed to extract file data" << std::endl;
		// 	return;
		// }
/*-------------------END OF PARSING FOR UPLOADING PICTURES--------------*/

		/*----------DRAFTING POST RESPONSE-------------------------------*/
		std::string	file = _request.getPath();
		if (file != "favicon.ico" && file != " " && !file.empty() && file != "" && file != "data/www/style.css")
			_response["body"] = openHtmlFile(file);
		else
		{
			_response["status"] = " 200 OK\r\n";
			_response["body"] = "GetResponse: Not handled yet\r\n";
		}
		_response["length"] = "Content-Length: ";
		_response["length"] += std::to_string(std::strlen(_response["body"].c_str()));
		_response["length"] += "\r\n";
		_response["type"] = "Content-Type: text/html\r\n"; //NEED TO PARSE
	
}

void	Response::DeleteResponse(void) {}

void	Response::executor(void) {
    //   nom=a&prenom=a&email=a%40q&ville=a
	std::cout << "EXECUTE POST REQUEST\n";
	// handleCgi();
}

void	Response::NotImplemented(void) {

	std::ifstream error("data/www/error/501.html");
    std::string content((std::istreambuf_iterator<char>(error)), (std::istreambuf_iterator<char>()));

	_response["status"] = " 501 Not Implemented\r\n";
	_response["body"] = content;
	_response["type"] = "Content-Type: text/html\r\n";
	_response["length"] = "Content-Length: ";
	_response["length"] += std::to_string(std::strlen(_response["body"].c_str()));
	_response["length"] += "\r\n";
	_response["connexion"] = "Connexion: close\r\n\r\n";	
}

std::string	Response::openHtmlFile(std::string f)
{
	std::ifstream file(f);
    if (!file.is_open())
	{
		_response["status"] = " 404 Not Found\r\n";
		return (openHtmlFile("data/www/error/404.html"));
	}
	else
	{
		_response["status"] = " 200 OK\r\n";
    	// read the contents of the file into a string variable
    	std::string content((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
		return content;
	}
}

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

/********************************* BadRequestError **********************************************/
void	Response::BadRequestError(void) {

	std::ifstream error("data/www/error/400.html");
    std::string content((std::istreambuf_iterator<char>(error)), (std::istreambuf_iterator<char>()));

	_response["status"] = " 400 Bad Request\r\n";
	_response["body"] = content;
	_response["type"] = "Content-Type: text/html\r\n";
	_response["length"] = "Content-Length: ";
	_response["length"] += std::to_string(std::strlen(_response["body"].c_str()));
	_response["length"] += "\r\n";
	_response["connexion"] = "Connexion: close\r\n\r\n";
}
/************************************************************************************************/

std::map<std::string, std::string>	Response::getMap() { return _response; }

Response::~Response() {}