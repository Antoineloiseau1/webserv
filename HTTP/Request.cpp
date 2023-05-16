#include "Request.hpp"
#include <fstream>

Request::Request(std::string request) {
	
	std::string line;
	std::istringstream iss(request);

	std::getline(iss, line);
	// Ignore whitepaces
	while(!line.empty() && line == "\r")
		getline(iss, line);

	std::istringstream first_line(line);
	first_line >> this->_initialRequestLine["type"];
	first_line >> this->_initialRequestLine["path"];
	this->_initialRequestLine["path"].erase(0, 1);
	first_line >> this->_initialRequestLine["version"];

	getline(iss, line);
	while(!line.empty() && line != "\r")
	{
		int delim = line.find_first_of(':');
		_headers[line.substr(0, delim)] = line.substr(delim + 2, line.size() - (delim + 2));
		getline(iss, line);
	}
	if (_initialRequestLine["type"].find("POST") != std::string::npos) {
		getline(iss, line);
		while(!line.empty())
		{	
			line += "\n";
			_body += line;
			line.clear();
			getline(iss, line);
		}
		if (_headers["Referer"].find("data/www/upload.html"))
			parsingBody();
	}
}

void saveImage(const std::string& imageData, const std::string& filePath) {
    std::ofstream file(filePath, std::ios::out | std::ios::binary);
    if (file) {
        file.write(imageData.data(), imageData.size());
        file.close();
        std::cout << "Image saved successfully!" << std::endl;
    } else {
        std::cerr << "Failed to open file for writing: " << filePath << std::endl;
    }
}

void	Request::parsingBody() {
/*---------------PARSING POST BODY FOR UPLOADING PICTURE REQUEST ONLY*/
		std::cout << "---- In parsing body\n";

		std::map<std::string, std::string>	img_data;

		std::string content_type = getHeaders()["Content-Type"];
		int loc_delim = content_type.find("boundary=") + 9;
		std::string	boundary = content_type.substr(loc_delim, content_type.size() - loc_delim);

		std::string line;
		std::istringstream iss(getBody());
		fflush(stdout);

		std::getline(iss, line);
		for(int i = 0; i < 2; i++) {
			int delim = line.find_first_of(':');
			img_data[line.substr(0, delim)] = line.substr(delim + 2, line.size() - (delim + 2));
			getline(iss, line);
			std::cout << "***** LINE = " <<	line << std::endl;
		}
	
		getline(iss, line);
		while(!line.empty())
		{	
			line += "\n";
			img_data["img_data"] += line;
			getline(iss, line);
		}
		std::cout << "***** IMG DATA size = " <<	img_data["img_data"].size() << std::endl;
		int loc = img_data["Content-Disposition"].find("filename=") + 9;
		std::string file_name = img_data["Content-Disposition"].substr(loc, img_data["Content-Disposition"].size() - loc); 
    	if (file_name.empty()) {
			std::cerr << "Failed to extract file data" << std::endl;
			return;
		}
		// std::string filePath = "/data/uploads/" + file_name;
		// saveImage(img_data["img_data"], filePath);
}

Request::~Request(void) {}

std::string	Request::getType() { return _initialRequestLine["type"]; }

std::string	Request::getPath() { return _initialRequestLine["path"]; }

std::string	Request::getVersion() { return _initialRequestLine["version"]; }

std::string	Request::getBody() { return _body; }

std::map<std::string, std::string>	Request::getHeaders() { return _headers; }