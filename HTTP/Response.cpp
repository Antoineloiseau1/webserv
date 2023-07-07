#include "Response.hpp"
#include <map>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <array>
#include <sys/stat.h>
#include <dirent.h>
#include "../utils.hpp"
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

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
	_request(request), _tmpPictFile(tmp_file), _firstTry(true), _isADirectory(false), _fileErrorDetected(false)
{
	setConfig();
	std::vector<std::string>	requestTypes = findMethods();

	enum		mtype {OTHER, GET, POST, DELETE, ERROR413};
	int			a = 0;
	size_t		i;

	for (i = 0; i != requestTypes.size(); i++)
	{
		if (request.getTypeStr() == requestTypes[i])
		{
			a = i + 1;
			if (i > 4 || a > 3)
			{
				a = 0;
				break;
			}
			break;
		}
	}
	if(i == requestTypes.size())
		a = 5;
	rootFile();
	std::string f = _file;
	if(f != "style.css" && f != _curRoute &&  f != "delete" && f != "data/www/gallery.html" && checkPermissions(f.substr(0, f.find_last_of("/")), f) == 1) {
		notFound404();
		return;
	}
	if (_request.badRequest)
		a = 42;
	if (!_request.badRequest && _request.noContent)
		a = 45;
	if (!_server.getData().getServers()[_curServer][_curRoute]["client_max_body_size"].empty()
		&& atoi(_request.getHeaders()["Content-Length"].c_str()) > atoi(_server.getData().getServers()[_curServer][_curRoute]["client_max_body_size"].c_str()))
		a = ERROR413;
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
			_server.deletePict("data/images/" + request.getFileName());
			RequestEntityTooLargeError();
			break;
		case 45:
			noContent204();
			break;
		default:
			if(_request.getTypeStr() == "DELETE" || _request.getTypeStr() == "POST" || _request.getTypeStr() == "GET" )
			{
				if(_request.getHeaders()["Content-Type"].empty())
					BadRequestError();
				else
					forbidden403();
			}
			else if (_request.getTypeStr() == "OPTIONS" || _request.getTypeStr() == "HEAD" || _request.getTypeStr() == "PUT" || _request.getTypeStr() == "TRACE" || _request.getTypeStr() == "CONNECT")
				NotImplemented();
			else
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

void	Response::generateAutoindex(std::string path)
{
	DIR				*fd;
	struct dirent	*currentFile;
	
	if (path.empty())
		path = "data/";
	fd = opendir(path.c_str());
	if(!fd)
	{
		std::cerr << "generateAutoindex: Cannot open " << path << std::endl;
		return internalServerError505();
	}
	_response["status"] = " 200 OK\r\n";
	_response["body"] = "<!DOCTYPE html>\n"
							"<html>\n"
							"<head>\n"
							"<title>";
	_response["body"] += path;
	_response["body"] += "</title\n>"
						"</head>\n"
						"<body>\n"
						"<header>\n"
						"<h1>";
	_response["body"] += path;
	_response["body"] += "</h1>\n"
						"<nav>\n"
						"<ul>\n";
	currentFile = readdir(fd);
	while(currentFile)
	{
		if(!strcmp(currentFile->d_name, ".") || !strcmp(currentFile->d_name, ".."))
		{
			currentFile = readdir(fd);
			continue;
		}
		_response["body"] += "<li><a href=\"/";
		_response["body"] += path;
		if(path[path.length() - 1] != '/')
			_response["body"] += "/";
		_response["body"] += currentFile->d_name;
		_response["body"] += "\">";
		_response["body"] += currentFile->d_name;
		_response["body"] += "</a></li>\n";
		currentFile = readdir(fd);
	}
	_response["body"] += "</ul>\n"
						"</nav>\n"
						"</header>\n"
						"</body>\n";
	_contentSize = std::strlen(_response["body"].c_str());
	_response["type"] = "text/html";
	closedir(fd);
}

void	Response::checkOpeningOfDirectory() {
	size_t i = 0;
	if (_server.getData().getServers()[_curServer][_curRoute].count("index") > 0) {
		while (1) {
			if (_server.getData().getServers()[_curServer][_curRoute].count("index_" + std::to_string(i)) < 1)
				break ;
			_response["body"] = openHtmlFile(_server.getData().getServers()[_curServer][_curRoute]["index_" + std::to_string(i)]);
			if (!_fileErrorDetected)
				break ;
			i++;
		}
	}
	else
		forbidden403();
}
			

void	Response::fillGetBody(std::string file) {
	if (file == "" || _isADirectory)
	{
		if (_server.getData().getServers()[_curServer][_curRoute]["autoindex"] == "on") {
			generateAutoindex(_request.getPath());
		}
		else if (_server.getData().getServers()[_curServer][_curRoute]["autoindex"] == "off"
			|| _server.getData().getServers()[_curServer][_curRoute]["autoindex"].empty()) {
			checkOpeningOfDirectory();
		}
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
							"<li><a href=\"milan.html\">Accueil</a></li>\n"
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
	_response["length"] += std::to_string(_contentSize);
	_response["length"] += "\r\n";
}

void	Response::fillGetType(std::string file) {
	_response["type"] = "Content-Type: text/html\r\n"; //Main case
	if (file.find("data/images/") != std::string::npos)
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
	//std::string validExtensions[] = {"html", "png", "jpeg", "css", "ico", "jpg"};

	if (cgiCase.empty())
		cgiCase = "cgi";
	if (cgiCase.find(".") == 0)
		cgiCase.erase(0, 1);
	if (extension == cgiCase)
		return 0;
	// for (int i = 0; i < 6; i++)
	// {
	// 	if (extension == validExtensions[i])
	// 		return i + 1;
	// }
	return 1;
}

int		Response::findServer()
{
	std::string line;
	for (size_t i = 0; i != _server.getData().getServers().size(); i++)
	{
		line = _request.getHeaders()["Host"];
		if (_server.getData().getServers()[i]["default"]["server_name"].empty())
			_server.getData().getServers()[i]["default"]["server_name"] = "localhost";
		if (_server.getData().getServers()[i]["default"]["listen"].empty())
			_server.getData().getServers()[i]["default"]["listen"] = "80";

		std::string serverSetup = _server.getData().getServers()[i]["default"]["server_name"] + ":" + _server.getData().getServers()[i]["default"]["listen"];
		if (!strncmp(line.c_str(), serverSetup.c_str(), serverSetup.length()))
			return i;
	}
	return 0;
}

void	Response::GetResponse(int fd) {
	
		int type = -1;
		if (_server.getData().getServers()[_curServer][_curRoute].count("redirect") > 0)
		{
			_response["status"] = " 301 Moved Permanently\r\n";
			_response["status"] += "Location: ";
			_response["status"] += _server.getData().getServers()[_curServer][_curRoute]["redirect"];
			_response["status"] += "\n";
			return ;
		}
		if (_file.empty())
			_file += "data/www/";
		if (!_file.empty())
		{
			std::string extension = getExtension(_file);
			type = isValid(extension, _server.getData().getServers()[_curServer][_curRoute]["cgi_extension"]);
			if (isADirectory(_file)) {
				type = 2;
				_isADirectory = true;
			}
		}
		else
			_file = "";
		if (type)
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
	if (!isValid(getExtension(file), _server.getData().getServers()[_curServer][_curRoute]["cgi_extension"]))
	{
		handleCgi(file, fd);
		return;
	}
	if (_request.isADataUpload == true) {
			std::ifstream sourceFile(_tmpPictFile, std::ios::in | std::ios::binary);
			std::string filePath = "data/images/" + _request.getFileName(); 
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
		DeleteResponse();
		return ;
	}
	else if (file != "favicon.ico" && file != " " && !file.empty() && file != "" && file != "data/www/style.css")
	{
		if (isADirectory(file))
			methodNotAllowed405();
		else {
			if(checkPermissions(file.substr(0, file.find_last_of('/')).c_str(), file) == 1)
				notFound404();
			else {
			_response["status"] = " 201 Created\r\n";
			_response["body"] = openHtmlFile("data/www/error/201.html");
			}
		}
	}
	if(file == "")
	{
			_response["status"] = " 201 Created\r\n";
			_response["body"] = openHtmlFile("data/www/error/201.html");
	}
	fillGetLength();
	_response["type"] = "Content-Type: text/html\r\n";
}

void	Response::DeleteResponse(void) {

	std::string file = _request.getFileToDelete();
	if(file.empty())
		file = _file;
	if(checkPermissions(file.substr(0, file.find_last_of('/')).c_str(), file) == 1)
		notFound404();
	else if(checkPermissions(file.substr(0, file.find_last_of('/')).c_str(), file) == 2)
		forbidden403();
	else {
		_server.deletePict(file);
		std::remove(file.c_str());
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
		_fileErrorDetected = true;
		_response["status"] = " 403 Forbidden\r\n";
		if (_server.getData().getServers()[_curServer][_curRoute].count("403") > 0)
			return openHtmlFile(_server.getData().getServers()[_curServer][_curRoute]["error_page"]);
		return openHtmlFile("data/www/error/403.html");
	}
	else if(permit == 1)
	{
		_fileErrorDetected = true;
		_response["status"] = " 404 Not Found\r\n";
		if (_server.getData().getServers()[_curServer][_curRoute].count("404") > 0) {
			if (_firstTry) {
				_firstTry = false;
				return openHtmlFile(_server.getData().getServers()[_curServer][_curRoute]["error_page"]);
			}
		}
		return openHtmlFile("data/www/error/404.html");
	}
	if (f.find("data/images/") != std::string::npos || f.find("favicon.ico") != std::string::npos)
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
		_fileErrorDetected = true;
		if (_server.getData().getServers()[_curServer][_curRoute].count("404") > 0)
			return openHtmlFile(_server.getData().getServers()[_curServer][_curRoute]["error_page"]);
		return openHtmlFile("data/www/error/404.html");
	}
}

void	Response::createCgiEnv()
{

		_env.push_back("REQUEST_METHOD=GET");
		_env.push_back("QUERY_STRING=" + _request.getHeaders()["formbody"]);
		_env.push_back("CONTENT_TYPE=text/html");
		_env.push_back(std::to_string(std::strlen(_env[1].c_str())));
		_env.push_back("HTTP_USER_AGENT=Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3");
		_env.push_back("REMOTE_ADDR=127.0.0.1");
}


void	Response::handleCgi(std::string file, int fd) {
	int pipefd[2];

	std::string arg1 = file;
	const char* cmd1_cstr = arg1.c_str();
	std::string arg2 = _request.getBody();
	const char* cmd2_cstr = arg2.c_str();
	char* const args[] = { const_cast<char*>(cmd1_cstr), const_cast<char*>(cmd2_cstr), NULL };
	FILE	*tmpFile = freopen("data/CGI/ocgi.html", "wr", stdout);

	if (pipe(pipefd) == -1) {
		fclose(tmpFile);
		std::remove("data/CGI/ocgi.html");
		tmpFile = freopen("/dev/tty", "w", stdout);
		return internalServerError505();
	}
	close(pipefd[0]);
	pid_t pid = fork();
	if (pid == -1) {
		close(pipefd[0]);
		close(pipefd[1]);
		fclose(tmpFile);
		std::remove("data/CGI/ocgi.html");
		tmpFile = freopen("/dev/tty", "w", stdout);
		return internalServerError505();
	}

	if (pid == 0) {  //Processus enfant 
		pipefd[1] = fd;
		createCgiEnv();
	
		char	*cgiEnv[_env.size() + 1];
		size_t	i;

		for (i = 0; i != _env.size(); i++)
			cgiEnv[i] = strdup(const_cast<const char *>(_env[i].c_str()));
		cgiEnv[i] = 0;

		if (execve(args[0], args, cgiEnv) == -1)
			internalServerError505();
	}
	else
	{
		ourSleepFunction(2);
		int result;
		int status;
		result = waitpid(pid, &status, WNOHANG);
		if(result == 0 || result == -1)
		{
			kill(pid, SIGTERM);
			fclose(tmpFile);
			tmpFile = freopen("/dev/tty", "w", stdout);
			std::remove("data/CGI/ocgi.html");
			close(pipefd[0]);
			close(pipefd[1]);
			return gatewayTimeout504();
		}
		else
		{
			if(WIFSIGNALED(status))
			{			
				kill(pid, SIGTERM);
				fclose(tmpFile);
				tmpFile = freopen("/dev/tty", "w", stdout);
				std::remove("data/CGI/ocgi.html");
				close(pipefd[0]);
				close(pipefd[1]);
				return internalServerError505();
			}
			FILE	*ocgi = fopen("data/CGI/ocgi.html", "r");
			fclose(tmpFile);
			tmpFile = freopen("/dev/tty", "w", stdout);
			fclose(ocgi);
			_response["status"] = " 200 OK\r\n";
			_response["body"] = openHtmlFile("data/CGI/ocgi.html");
			fillGetLength();
			fillGetType("data/CGI/ocgi.html");
			std::remove("data/CGI/ocgi.html");
			close(pipefd[1]);
			close(pipefd[0]);
		}
	}
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
	std::remove(_tmpPictFile.c_str());
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

int	Response::checkPermissions(std::string directory, std::string file)
{
	struct dirent	*currentFile;
	struct stat		sfile;
	DIR				*fd;

	if (file.empty())
		return 2;
	fd = opendir(directory.c_str());
	if (fd == NULL)
	{
		std::cerr << "checkPermissions: Couldn't open " << directory << std::endl;
		return 1;
	}
	currentFile = readdir(fd);
	
	while(currentFile)
	{
		if (isADirectory(file)) {
			closedir(fd);
			return 2;
		}
		if (file == (directory + "/" + std::string(currentFile->d_name)))
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
		if (isADirectory(file)) {
			closedir(fd);
			return 2;
		}
		else {
			closedir(fd);
			return 1; //file not found
		}
	}
	return 0;
}

void	Response::methodNotAllowed405() {
	std::string file = "data/www/error/405.html";
	
	_response["status"] = " 405 Not Allowed\r\n";
	if (_server.getData().getServers()[_curServer][_curRoute].count("404") > 0)
		file = _server.getData().getServers()[_curServer][_curRoute]["error_page"];
	_response["body"] = openHtmlFile(file);
}

void	Response::notFound404() {
	std::string file = "data/www/error/404.html";
	
	_response["version"] = _request.getVersion();
	_response["connexion"] = "Connexion: close\r\n\r\n";
	_response["status"] = " 404 Not Found\r\n";
	if (_server.getData().getServers()[_curServer][_curRoute].count("404") > 0)
		file = _server.getData().getServers()[_curServer][_curRoute]["error_page"];
	_response["body"] = openHtmlFile(file);
	_response["type"] = "Content-Type: text/html\r\n";
	fillGetLength();
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

void	Response::gatewayTimeout504(void) {
	std::string file = "data/www/error/504.html";
	
	_response["status"] = " 504 Gateway Timeout\r\n";
	if (_server.getData().getServers()[_curServer][_curRoute].count("504") > 0)
		file = _server.getData().getServers()[_curServer][_curRoute]["error_page"];
	_response["body"] = openHtmlFile(file);
}

std::map<std::string, std::string>	Response::getMap() { return _response; }

std::string	Response::getFile(void) {return _file; }

Response::~Response() {}
