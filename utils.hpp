#ifndef UTILS_HPP
# define UTILS_HPP

#include <string>

std::string urlDecode(const std::string& encoded);
bool		isADirectory(std::string const path);
int			isAllDigit(std::string s);

#endif