#include "utils.hpp"
#include <dirent.h>
#include <sstream>
#include <iomanip>

std::string urlEncode(const std::string& input) {
    std::ostringstream encoded;
    encoded.fill('0');
    encoded << std::hex;

    for (std::string::const_iterator it = input.begin(); it != input.end(); ++it) {
        char c = *it;

        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded << c;
        }
        else if (c == ' ')
            encoded << '%' << std::setw(2) << int((unsigned char)c);
        else
            encoded << '%' << std::setw(2) << int((unsigned char)c);
    }

    return encoded.str();
}

std::string urlDecode(const std::string& encoded) {
    std::string decoded;
   size_t i = 0;

    while (i < encoded.length()) {
        if (encoded[i] == '%') {
            int hexValue;
            sscanf(encoded.substr(i + 1, 2).c_str(), "%x", &hexValue);
            decoded += static_cast<char>(hexValue);
            i += 3;
        } else {
            decoded += encoded[i];
            ++i;
        }
    }
    return decoded;
}

bool	isADirectory(std::string const path)
{
	DIR* dir = opendir(path.c_str());
    if (dir) {
        closedir(dir);
        return true;
    }
    return false;
}

int	isAllDigit(std::string s) {
	for (unsigned int i = 0; i < s.size(); i++) {
		if (!isdigit(s[i]))
			return 0;
	}
	return 1;
}

void    ourSleepFunction(unsigned int sec) {
    time_t start = time(NULL);
    while(time(NULL) - start < sec)
        continue;
}