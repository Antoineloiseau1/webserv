#include "Response.hpp"

Response::Response(Request &request) : _request(request)
{
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

Response::~Response() {}

std::string	Response::openHtmlFile(std::string f)
{
	std::ifstream file(f);
	std::cout << "-- TEST 1 = " << f << "\n";
    if (!file.is_open())
    {
		std::cout << "-- TEST 1\n";
		return (openHtmlFile("data/www/error.html"));
    }

    // read the contents of the file into a string variable
    std::string content((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
	if (content.empty())
	{
		std::cout << "-- TEST 22\n";
		return (openHtmlFile("data/www/error.html"));
    }
	return content;
}

std::map<std::string, std::string>	Response::getMap() { return _response; }

GetResponse::GetResponse(Request request) : Response(request) {}

GetResponse::~GetResponse() {}

void	GetResponse::executor() {

	std::cout << "EXECUTE GET REQUEST\n";
}

PostResponse::PostResponse(Request request) : Response(request) {}

PostResponse::~PostResponse() {}

void	PostResponse::executor() {
	std::cout << "EXECUTE POST REQUEST\n";
}

DeleteResponse::DeleteResponse(Request request) : Response(request) {}

DeleteResponse::~DeleteResponse() {}

void	DeleteResponse::executor() {
	std::cout << "EXECUTE Delete REQUEST\n";
}