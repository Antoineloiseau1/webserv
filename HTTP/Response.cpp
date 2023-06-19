#include "Response.hpp"
#include <map>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <array>
#include <sys/stat.h>
#include <dirent.h>

bool	isValid(std::vector<std::string> toBeValidated, std::vector<std::string> example)
{
	for (size_t i = 0; i != toBeValidated.size(); i++)
	{
		for (size_t j = 0; i != example.size(); j++)
		{
			if (toBeValidated[i] == example[j])
				break;
			if (j == example.size() - 1)
				return false;
		}
	}
	return true;
}

std::vector<std::string>	Response::findMethods()
{
	_file = _request.getPath();

	std::vector<std::string>	methodTab;
	std::vector<std::string>	knownMethods;
	std::string					tmp;
	std::string					methodsList = _server.getData().getServers()[_curServer][_curRoute]["limit_except"];

	knownMethods.push_back("GET");
	knownMethods.push_back("POST");
	knownMethods.push_back("DELETE");

	knownMethods.push_back("HEAD");   //don't know what it is but it was here
	knownMethods.push_back("OPTIONS");
	knownMethods.push_back("PUT");
	knownMethods.push_back("TRACE");
	knownMethods.push_back("CONNECT");

	if (methodsList.empty())
		return knownMethods;

	for (size_t	i = 0; methodsList[i]; i++)
	{
		if (isspace(methodsList[i]) && !tmp.empty())
		{
			methodTab.push_back(tmp);
			tmp.clear();
		}
		else if (!isspace(methodsList[i]))
			tmp += methodsList[i];
	}
	if (!tmp.empty())
		methodTab.push_back(tmp);

	if (!isValid(methodTab, knownMethods))
		throw(UnknownDataException());
	for (std::vector<std::string>::iterator it = methodTab.begin(); it != methodTab.end(); it++) {
		std::cout << "\nMETHOD TAB = " << *it << std::endl;
	}
	return methodTab;
}

void	Response::rootFile()
{
	std::string	curRoute = findRoute(_request.getPath());
	std::string rootedTo = _server.getData().getServers()[_curServer][curRoute]["root"];
	if (rootedTo.empty())
		return ;

	int pos = _file.find(curRoute);
	_file.replace(pos, curRoute.length(), rootedTo);
}

void	Response::setConfig()
{
	_file = urlDecode(_request.getPath());
	_curServer = findServer();
	_curRoute = findRoute(_file);

	std::string redirection = _server.getData().getServers()[_curServer][_curRoute]["return"];
	if (!redirection.empty()) //change Host: localhost:post to Host: redirection:port
	{
		int pos = _request.getHeaders()["Host"].find(":");
		_request.getHeaders()["Host"].replace(0, pos, redirection);
		_curServer = findServer();
		_curRoute = findRoute(_file);
	}
}

Response::Response(Request &request, Server &server, std::string tmp_file, int fd) : _server(server),
	_request(request), _tmpPictFile(tmp_file), _firstTry(true)
{
	setConfig();
	std::vector<std::string>	requestTypes = findMethods();

	enum		mtype { OTHER, GET, POST, DELETE, ERROR413 };
	int			a = 0;

	for (size_t i = 0; i != requestTypes.size(); i++)
	{
		if (request.getTypeStr() == requestTypes[i])
		{
			a = i + 1;
			if (i > 4)
			{
				a = 0;
				break;
			}
			break;
		}
		// a = i;
	}

	rootFile();
	
	// std::cout <<"aled " << _file << std::endl;
	std::cout << "\n\n\n\n\n\n";
	std::cout << _request.getHeaders()["Content-Length"] << std::endl;
   std::cout << _server.getData().getServers()[_curServer][_curRoute]["client_max_body_size"] << std::endl;
	if (!_server.getData().getServers()[_curServer][_curRoute]["client_max_body_size"].empty()
		&& atoi(_request.getHeaders()["Content-Length"].c_str()) > atoi(_server.getData().getServers()[_curServer][_curRoute]["client_max_body_size"].c_str()))
		a = ERROR413;
	std::cout << "\n\n JE SUIS ICI 000 !!!!!! a = "<< a << "\n\n";
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
		case ERROR413:
			RequestEntityTooLargeError();
			break;
		default:
			BadRequestError();
	}
	_response["version"] = _request.getVersion();
	_response["connexion"] = "Connexion: close\r\n\r\n";
}

std::string	Response::findRoute(std::string const file)
{
	int s = _curServer;

		for (std::map<std::string, std::map<std::string, std::string> >::iterator route = _server.getData().getServers()[s].begin(); route != _server.getData().getServers()[s].end(); route++)
	{
		if (!strncmp(route->first.c_str(), file.c_str(), route->first.length()))
			return route->first;
	}
	return "default";
}

void	Response::fillGetBody(std::string file) {
	if (file == "")
	{
		if (_server.getData().getServers()[_curServer][_curRoute]["autoindex"] == "on")
			_response["body"] = openHtmlFile("data/www/index.html");
		else if (_server.getData().getServers()[_curServer][_curRoute]["autoindex"] == "off" || _server.getData().getServers()[_curServer][_curRoute]["autoindex"].empty())
			_response["body"] = openHtmlFile("data/www/manon.html");
		else
		{
			std::cerr << "Error in config file\n"; //maybe better error handling later
			exit(1);
		}
	}
	else if(file.substr(file.find_last_of("/") + 1) == "style.css" || file.empty())
	{
		_response["status"] = " 200 OK\r\n";
		_response["body"] = openHtmlFile("data/www/style.css");
		_response["content-type"] = "text/css";
	}
	else if (file == "data/www/gallery.html")
	{
		_response["body"] = "<!DOCTYPE html>\n"
							"<html>\n"
							"<head>\n"
							"<title>Picture Gallery</title>\n"
							"</head>\n"
							"<nav>\n"
							"<ul>\n"
							"<li><a href=\"manon.html\">Accueil</a></li>\n"
							"</ul>\n"
							"</nav>\n"
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
		_response["body"] = openHtmlFile(file);
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
	if (file.find(".css") != std::string::npos)
		_response["type"] = "Content-type: text/css\r\n";
}

std::string	getExtension(std::string &file)
{
	int i;
	try{
		i = file.find_last_of(".");
	}
	catch (std::exception &e)
	{
		std::cerr << "Exception caugth : " << e.what() << std::endl;
		return "";
	}
	std::string extension = file.substr(i + 1);
	return extension;
}

int	isValid(std::string const extension, std::string cgiCase)
{
	std::string validExtensions[] = {"html", "png", "jpeg", "css", "ico", "jpg"};

	if (cgiCase.find(".") == 0)
		cgiCase.erase(0, 1);
	if (extension == cgiCase)
		return 0;
	for (int i = 0; i < 6; i++)
	{
		if (extension == validExtensions[i])
			return i + 1;
	}
	return -42;
}

//A METTRE DANS UTILS
bool	isADirectory(std::string const path)
{
	DIR* dir = opendir(path.c_str());
    if (dir) {
        closedir(dir);
        return true;
    }
    return false;
}

int		Response::findServer()
{
	std::string line;
	for (size_t i = 0; i != _server.getData().getServers().size(); i++)
	{
		std::string serverSetup = _server.getData().getServers()[i]["default"]["server_name"] + ":" + _server.getData().getServers()[i]["default"]["listen"];
		if (!strncmp(line.c_str(), serverSetup.c_str(), serverSetup.length()))
			return i;
	}
	return 0;
}

void	Response::GetResponse(int fd) {
	
		int type = -1;
		if (!_file.empty())
		{
			std::string extension = getExtension(_file);
			type = isValid(extension, _server.getData().getServers()[_curServer][_curRoute]["cgi_extension"]);
			if (isADirectory(_file)) {
				type = 2;
				_file = "";
			}
		}
		else
			_file = "";
		if (type == -42) {
			BadRequestError();
		}
		else if (type)
		{
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
	std::cout << "\n\n JE SUIS LA !!!!!!\n\n";
	if (!isValid(getExtension(file), _server.getData().getServers()[_curServer][_curRoute]["cgi_extension"]))
	{
		std::cout << "\n1111111111\n";
		handleCgi(file, fd); //maybe a bool
		return;
	}
	if (_request.isADataUpload == true) {
		std::cout << "\n222222222222\n";
			std::ifstream sourceFile(_tmpPictFile, std::ios::in | std::ios::binary);
			std::string filePath = "uploads/" + _request.getFileName(); 
			std::ofstream destFile(filePath, std::ios::out | std::ios::binary);

			if (sourceFile.is_open() && destFile.is_open()) {
				destFile << sourceFile.rdbuf();
				std::cerr << "File copied successfully." << std::endl;
			}
			else
				std::cerr << "Failed to open the file." << std::endl;
    		sourceFile.close();
    		destFile.close();
	}
	if (_request.isDelete) {
		std::cout << "\n33333333\n";
		DeleteResponse();
		return ;
	}
	else if (file != "favicon.ico" && file != " " && !file.empty() && file != "" && file != "data/www/style.css")
	{ 
		std::cout << "\n44444444\n";
		_response["status"] = " 201 Created\r\n";
		_response["body"] = openHtmlFile("data/www/error/201.html");
		// return;
	}
	fillGetLength();
	_response["type"] = "Content-Type: text/html\r\n";// tjrs que du html pour le moment
	std::cout << "\n5555555\n";
}

void	Response::DeleteResponse(void) {

	_file = _request.getFileToDelete();
	// if(_file.empty())
	// 	_file = urlDecode(_file);
	if(checkPermissions(_file.substr(0, _file.find_last_of('/')).c_str(), _file) == 1)
		notFound404();
	else if(checkPermissions(_file.substr(0, _file.find_last_of('/')).c_str(), _file) == 2)
		forbidden403();
	else {
		_server.deletePict(_file);
		ok200();
		std::cerr << "File deleted successfully" << std::endl;
	}
}

std::string	Response::openHtmlFile(std::string f)
{
    std::ifstream file;

	int permit = checkPermissions(f.substr(0, f.find_last_of('/')).c_str(), f);
	if(permit == 2)
	{
		_response["status"] = " 403 Forbidden\r\n";
		if (_server.getData().getServers()[_curServer][_curRoute].count("403") > 0)
			return openHtmlFile(_server.getData().getServers()[_curServer][_curRoute]["error_page"]);
		return openHtmlFile("data/www/error/403.html");
	}
	else if(permit == 1)
	{
		_response["status"] = " 404 Not Found\r\n";
		if (_server.getData().getServers()[_curServer][_curRoute].count("404") > 0) {
			if (_firstTry) {
				_firstTry = false;
				return openHtmlFile(_server.getData().getServers()[_curServer][_curRoute]["error_page"]);
			}
		}
		return openHtmlFile("data/www/error/404.html");
	}
	if (f.find("uploads/") != std::string::npos || f.find("favicon.ico") != std::string::npos)
		file.open(f, std::ios::binary);
	else
		file.open(f);
	if (file.is_open()) {
		struct stat fileInfo;
		if (stat(f.c_str(), &fileInfo) == 0)
			_contentSize = static_cast<size_t>(fileInfo.st_size);
		else
			std::cerr << "Failed to determine the file size." << std::endl;
		std::string content((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));//  	 read the contents of the file into a string variable
		return content;
	}
	else {
		_response["status"] = " 404 Not Found\r\n";
		if (_server.getData().getServers()[_curServer][_curRoute].count("404") > 0)
			return openHtmlFile(_server.getData().getServers()[_curServer][_curRoute]["error_page"]);
		return openHtmlFile("data/www/error/404.html");
	}
}

void	Response::createCgiEnv()
{

		_env[0] = "REQUEST_METHOD=GET";
		_env[1] = "QUERY_STRING=";
		_env[1] += _request.getHeaders()["formbody"];
		_env[2] = "CONTENT_TYPE=text/html";
		_env[3] = std::to_string(std::strlen(_env[1].c_str()));
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
		return internalServerError505();
	}
	close(pipefd[0]);

	pid_t pid = fork();
	if (pid == -1) {
		close(pipefd[0]);
		close(pipefd[1]);
		return internalServerError505();
	}

	if (pid == 0) {  //Processus enfant 
		pipefd[1] = fd;

		createCgiEnv();
		char *cgiEnv[_env.size() + 1];
		size_t i;
		for (i = 0; i != _env.size(); i++)
			cgiEnv[i] = strdup(const_cast<const char *>(_env[i].c_str()));
		cgiEnv[i] = 0;
		dup2(pipefd[1], STDOUT_FILENO);
		if (execve(args[0], args, cgiEnv) == -1)
			internalServerError505();
	}
	close(pipefd[1]);
	close(pipefd[0]);
}

std::string	Response::buildResponse(void) {
	return (_response["version"] + _response["status"] + _response["type"] + _response["length"] + _response["connexion"] + _response["body"]);
}

/********************************* BadRequestError **********************************************/
void	Response::BadRequestError(void) {

	std::string file = "data/www/error/400.html";
	
	if (_server.getData().getServers()[_curServer][_curRoute].count("400") > 0)
		file = _server.getData().getServers()[_curServer][_curRoute]["error_page"];

	std::ifstream error(file);
    std::string content((std::istreambuf_iterator<char>(error)), (std::istreambuf_iterator<char>()));

	_response["status"] = " 400 Bad Request\r\n";
	_response["body"] = content;
	_response["type"] = "Content-Type: text/html\r\n";
	_response["length"] = "Content-Length: ";
	_response["length"] += std::to_string(std::strlen(_response["body"].c_str()));
	_response["length"] += "\r\n";
}
/************************************************************************************************/

void	Response::RequestEntityTooLargeError(void) {

	std::string file = "data/www/error/413.html";
	
	if (_server.getData().getServers()[_curServer][_curRoute].count("413") > 0)
		file = _server.getData().getServers()[_curServer][_curRoute]["error_page"];

	std::ifstream error(file);
    std::string content((std::istreambuf_iterator<char>(error)), (std::istreambuf_iterator<char>()));

	_response["status"] = " 413 Request Entity Too Large\r\n";
	_response["body"] = content;
	_response["type"] = "Content-Type: text/html\r\n";
	_response["length"] = "Content-Length: ";
	_response["length"] += std::to_string(std::strlen(_response["body"].c_str()));
	_response["length"] += "\r\n";

	if (std::remove(_tmpPictFile.c_str()))
		std::cerr << "error: Failed to delete file.\n";
}

/********************************** NotImplemented **********************************************/
void	Response::NotImplemented(void) {
	std::string file = "data/www/error/501.html";
	
	if (_server.getData().getServers()[_curServer][_curRoute].count("501") > 0)
		file = _server.getData().getServers()[_curServer][_curRoute]["error_page"];
		
	std::ifstream error(file);
    std::string content((std::istreambuf_iterator<char>(error)), (std::istreambuf_iterator<char>()));

	_response["status"] = " 501 Not Implemented\r\n";
	_response["body"] = content;
	_response["type"] = "Content-Type: text/html\r\n";
	_response["length"] = "Content-Length: ";
	_response["length"] += std::to_string(std::strlen(_response["body"].c_str()));
	_response["length"] += "\r\n";
}
/************************************************************************************************/

int	Response::checkPermissions(const char *directory, std::string file)
{
	struct dirent	*currentFile;
	struct stat		sfile;
	DIR				*fd;

	std::cout << "dir " << directory << " file " << file << std::endl;
	fd = opendir(directory);
	if (fd == NULL)
	{
		std::cerr << "checkPermissions: Couldn't open " << directory << std::endl;
		return 1;
	}
	currentFile = readdir(fd);
	while(currentFile)
	{
		if (file == (std::string(directory) + "/" + std::string(currentFile->d_name)))
		{	
			if (stat(file.c_str(), &sfile) == -1)
			{
				closedir(fd);
				return 1;
			}
			if(!(sfile.st_mode & S_IRUSR))
			{
				closedir(fd);
				return 2;
			}
			else
			{
				closedir(fd);
				return 0;
			}
		}
		currentFile = readdir(fd);
	}
	if(currentFile == NULL) {
		closedir(fd);
		return 1; //file not found		
	}
	return 0;
}

void	Response::notFound404() {
	std::string file = "data/www/error/404.html";
	
	_response["status"] = " 404 Not Found\r\n";
	if (_server.getData().getServers()[_curServer][_curRoute].count("404") > 0)
		file = _server.getData().getServers()[_curServer][_curRoute]["error_page"];
	_response["body"] = openHtmlFile(file);
}

void	Response::ok200() {
	_response["status"] = " 200 OK\r\n";
	_response["body"] = openHtmlFile("data/www/error/200.html");
}

void	Response::forbidden403() {
	std::string file = "data/www/error/403.html";
	
	_response["status"] = " 403 Forbidden\r\n";
	if (_server.getData().getServers()[_curServer][_curRoute].count("403") > 0)
		file = _server.getData().getServers()[_curServer][_curRoute]["error_page"];
	_response["body"] = openHtmlFile(file);
}

void	Response::noContent204() {
	std::string file = "data/www/error/204.html";
	
	_response["status"] = " 204 No Content\r\n";
	if (_server.getData().getServers()[_curServer][_curRoute].count("204") > 0)
		file = _server.getData().getServers()[_curServer][_curRoute]["error_page"];
	_response["body"] = openHtmlFile(file);
	_response["type"] = "Content-Type: text/html\r\n";
	fillGetLength();
}

void	Response::internalServerError505(void) {
	std::string file = "data/www/error/505.html";
	
	_response["status"] = " 505 Internal Server Error\r\n";
	if (_server.getData().getServers()[_curServer][_curRoute].count("505") > 0)
		file = _server.getData().getServers()[_curServer][_curRoute]["error_page"];
	_response["body"] = openHtmlFile(file);
}

std::map<std::string, std::string>	Response::getMap() { return _response; }

std::string	Response::getFile(void) {return _file; }

Response::~Response() {}
