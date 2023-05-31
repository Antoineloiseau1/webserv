#include "Response.hpp"
#include <map>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <array>
#include <sys/stat.h>

Response::Response(Request &request, Server &server, std::string tmp_file, int fd) : _server(server),
	_request(request), _tmpPictFile(tmp_file)
{
	std::string	type[] = { "GET", "POST", "DELETE", "HEAD", "OPTIONS", "PUT", "TRACE", "CONNECT" };
	enum		mtype { GET, POST, DELETE, OTHER };
	int			a = 0;

	for (int i = 0; i < 8; i++)		// Possibilité de faire plus propre ?
	{
		if (request.getTypeStr() == type[i])
		{
			a = i;
			if (i > 2)
			{
				a = 3;
				break;
			}
			break;
		}
		a = i;
	}
	switch (a)
	{
		case GET:
			GetResponse(fd);
			break;
		case POST:
			PostResponse(fd);
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

void	Response::fillGetBody(std::string file) {
	// if (file.find("data/www/") == std::string::npos)
	// 	file = "data/www/" + file;
	if (file == "")
	{
		if (_server.getData().getData()["autoindex"] == "on")
			_response["body"] = openHtmlFile("data/www/index.html");
		else if (_server.getData().getData()["autoindex"] == "off" || _server.getData().getData()["autoindex"].empty())
			_response["body"] = openHtmlFile("data/www/manon.html");
		else
		{
			std::cout << "Error in config file\n"; //maybe better error handling later
			exit(1);
		}
	}
	else if(file == "style.css" || file.empty())
	{
		_response["status"] = " 204 No Content\r\n";
		_response["body"] = "";
	}
	else if (file == "data/www/gallery.html")
	{
		_response["body"] = "<!DOCTYPE html>\n"
							"<html>\n"
							"<head>\n"
							"<title>Picture Gallery</title>\n"
							"</head>\n"
							"<body>\n"
							"<h1>Picture Gallery</h1>\n";
		// Generate the HTML code for each picture
		for (std::vector<std::string>::iterator it = _server.pictPaths.begin(); it != _server.pictPaths.end() ; it++) {
			_response["body"] += "<div class=\"picture\">\n";
			_response["body"] += "<img src=\"/" +*it + "\" alt=\"Picture\">\n"; //on rajoute un '/' devant le path ici pour que ca marche
			_response["body"] += "<form action=\"/delete\" method=\"POST\">\n";
			_response["body"] += "<input type=\"hidden\" name=\"image\" value=\"" + *it + "\">\n";
			_response["body"] += "<input type=\"submit\" value=\"Delete\">\n";
			_response["body"] += "</form>\n";
			_response["body"] += "</div>\n";
		}
		_response["body"] += "</body>\n"
							"</html>\n";
		_contentSize = std::strlen(_response["body"].c_str());
	}
	else
	{
		_response["body"] = openHtmlFile(file);
	}

}

void	Response::fillGetLength() {
	_response["length"] = "Content-Length: ";
	// _response["length"] += std::to_string(std::strlen(_response["body"].c_str()));
	_response["length"] += std::to_string(_contentSize);
	_response["length"] += "\r\n";
}

void	Response::fillGetType(std::string file) {
	_response["type"] = "Content-Type: text/html\r\n"; //Main case
	if (file.find("uploads/") != std::string::npos)
	{
		std::string	ext = file.substr(file.find_last_of('.') + 1, file.size() - file.find_last_of('.') - 1);
		if (ext == "jpeg" || ext == "jpg")
			_response["type"] = "Content-Type: image/jpeg\r\n";
		if (ext == "png")
			_response["type"] = "Content-Type: image/png\r\n";
	}
	if (file.find("favicon.ico") != std::string::npos)
		_response["type"] = "Content-Type: image/x-icon\r\n";
}

std::string	getExtension(std::string &file)
{
	int i;
	try{
		i = file.find_last_of(".");
	}
	catch (std::exception &e)
	{
		std::cout << "Exception caugth : " << e.what() << std::endl;
		return "";
	}
	std::string extension = file.substr(i + 1);
	return extension;
}

int	isValid(std::string const extension, std::string cgiCase)
{
	std::string validExtensions[] = {"html", "png", "jpeg", "css", "ico"};

	if (cgiCase.find(".") == 0)
		cgiCase.erase(0, 1);
	std::cout << cgiCase << std::endl;
	std::cout << extension << std::endl;
	if (extension == cgiCase)
		return 0;
	for (int i = 0; i < 5; i++)
	{
		if (extension == validExtensions[i])
			return i + 1;
	}
	return -42;
}

void	Response::GetResponse(int fd) {
	
		_file = _request.getPath();
		int type = -1;
		if (!_file.empty())
		{
			std::string extension = getExtension(_file);
			type = isValid(extension, _server.getData().getData()["cgi_extension"]);
		}
		else
			_file = "data/www/index.html";
		if (type == -42)
			BadRequestError();
		else if (type)
		{
			if (type == 1)
			{
				if (_file.find("data/www/") == std::string::npos)
					_file = "data/www/" + _file;
			}
			_response["status"] = " 200 OK\r\n"; //Main case, updated when event in the building of response
			fillGetBody(_file);
			fillGetLength();
			fillGetType(_file);
		}
		else
			handleCgi(_file, fd);
}


void	Response::PostResponse(int fd) {
	std::string	file = _request.getPath();
	if (!isValid(getExtension(file), _server.getData().getData()["cgi_extension"]))
	{
		handleCgi(file, fd); //maybe a bool
		return; //a ne pas supprimer ??
	}
	if (_request.isADataUpload == true) {
			std::ifstream sourceFile(_tmpPictFile, std::ios::in | std::ios::binary);
			std::string filePath = "uploads/" + _request.getFileName(); 
			std::ofstream destFile(filePath, std::ios::out | std::ios::binary);

			if (sourceFile.is_open() && destFile.is_open()) {
				destFile << sourceFile.rdbuf();
				std::cout << "File copied successfully." << std::endl;
			}
			else
				std::cerr << "Failed to open the file." << std::endl;
    		sourceFile.close();
    		destFile.close();
	}
	if (_request.isDelete)
		DeleteResponse();
	else if (file != "favicon.ico" && file != " " && !file.empty() && file != "" && file != "data/www/style.css")
	{ ;
		_response["status"] = " 201 Created\r\n";
		_response["body"] = openHtmlFile("data/www/error/201.html");
		std::cout << "CGI" << std::endl;
		return; //a ne pas supprimer ??
	}
	fillGetLength();
	_response["type"] = "Content-Type: text/html\r\n";// tjrs que du html pour le moment
}

void	Response::DeleteResponse(void) {
	_request.setFileToDelete(_request.getPath().c_str());
	if (std::remove(_request.getFileToDelete().c_str()) != 0) {
		std::cerr << "Failed to delete file: " << std::endl;
		_response["status"] = " 404 Not Found\r\n";
		_response["body"] = openHtmlFile("data/www/error/404.html");
	} else {
		_server.deletePict(_request.getFileToDelete());
		std::cout << "File deleted successfully" << std::endl;
		_response["status"] = " 200 OK\r\n";
		_response["body"] = openHtmlFile("data/www/success.html");
	}
}

std::string	Response::openHtmlFile(std::string f) /* PROBLEME SI CEST UNE IMAGE A OUVRIR !!*/
{
    std::ifstream file;

	if (f.find("uploads/") != std::string::npos || f.find("favicon.ico") != std::string::npos)
		file.open(f, std::ios::binary);
	else
		file.open(f);
	if (file.is_open()) {
		struct stat fileInfo;
		if (stat(f.c_str(), &fileInfo) == 0)
			_contentSize = static_cast<size_t>(fileInfo.st_size);
		else
			std::cout << "Failed to determine the file size." << std::endl;
		std::string content((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));//  	 read the contents of the file into a string variable
		return content;
	}
	else {
		_response["status"] = " 404 Not Found\r\n";
		return (openHtmlFile("data/www/error/404.html"));
	}
}

void	Response::createCgiEnv()
{

		_env[0] = "REQUEST_METHOD=GET";
		_env[1] = "QUERY_STRING=nom=";
		_env[1] += _request.getHeaders()["formbody"];
		_env[2] = "CONTENT_TYPE=text/html";
		_env[3] ="CONTENT_LENGTH=500";
		_env[4] = "HTTP_USER_AGENT=Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3";
		_env[5] = "REMOTE_ADDR=127.0.0.1";
}


void	Response::handleCgi(std::string file, int fd) {
	int pipefd[2];

	std::string arg1 = file;
	const char* cmd1_cstr = arg1.c_str();
	std::string arg2 = _request.getBody();
	const char* cmd2_cstr = arg2.c_str();
	char* const args[] = { const_cast<char*>(cmd1_cstr), const_cast<char*>(cmd2_cstr), NULL };

	if (pipe(pipefd) == -1) {
		std::cerr << "Error 500\n";// antoine oskour
		return ;
	}
	close(pipefd[0]);

	pid_t pid = fork();
	if (pid == -1) {
		std::cerr << "Error 500\n"; // antoine aled
		close(pipefd[0]);
		close(pipefd[1]);
		return ;
	}

	if (pid == 0) {  //Processus enfant 
		pipefd[1] = fd;

		createCgiEnv();
		char *cgiEnv[_env.size() + 1];
		size_t i;
		for (i = 0; i != _env.size(); i++)
		{
			std::cout << strdup(const_cast<const char *>(_env[i].c_str())) << std::endl;
			cgiEnv[i] = strdup(const_cast<const char *>(_env[i].c_str()));
		}
		cgiEnv[i] = 0;
		dup2(pipefd[1], STDOUT_FILENO);
		if (execve(args[0], args, cgiEnv) == -1)
		{
			std::cerr << "error: 500(?)" << strerror(errno) << std::endl; //antoine error
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


/********************************** NotImplemented **********************************************/
void	Response::NotImplemented(void) {

	std::ifstream error("data/www/error/501.html");
    std::string content((std::istreambuf_iterator<char>(error)), (std::istreambuf_iterator<char>()));

	_response["status"] = " 501 Not Implemented\r\n";
	_response["body"] = content;
	_response["type"] = "Content-Type: text/html\r\n";
	_response["length"] = "Content-Length: ";
	_response["length"] += std::to_string(std::strlen(_response["body"].c_str()));
	_response["length"] += "\r\n";
}
/************************************************************************************************/

std::map<std::string, std::string>	Response::getMap() { return _response; }

std::string	Response::getFile(void) {return _file; }

Response::~Response() {}
