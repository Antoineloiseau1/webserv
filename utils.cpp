#include "utils.hpp"

/* A METTRE DANS UTILS*/
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