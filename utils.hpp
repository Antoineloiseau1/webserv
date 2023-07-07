#ifndef UTILS_HPP
# define UTILS_HPP

#include <string>

std::string urlDecode(const std::string& encoded);
std::string urlEncode(const std::string& input);
bool		isADirectory(std::string const path);
int			isAllDigit(std::string s);
void    	ourSleepFunction(unsigned int sec);

#endif