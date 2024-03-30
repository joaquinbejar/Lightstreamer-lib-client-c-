/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 30/3/24
 ******************************************************************************/

/*******************************************************************************
 Copyright (c) 2024.

 This program is free software: you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the
 Free Software Foundation, either version 3 of the License, or (at your
 option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 Public License for more details.

 You should have received a copy of the GNU General Public License along
 with this program. If not, see <https://www.gnu.org/licenses/>..
 ******************************************************************************/

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_LSUTILS_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_LSUTILS_HPP
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

namespace lightstreamer::util {

/**
 * @class LsUtils
 * @brief Utility class for various string and URI handling functions.
 */
    class LsUtils {
public:
    // Assuming a simple URI struct for demonstration. In a real application, consider a comprehensive URI parsing solution.
    struct Uri {
        std::string scheme;
        std::string host;
        int port = -1;

        Uri(const std::string& uri) {
            // Simple parsing assuming format: scheme://host:port
            auto schemeEnd = uri.find("://");
            if (schemeEnd != std::string::npos) {
                scheme = uri.substr(0, schemeEnd);
                auto hostStart = schemeEnd + 3;
                auto portStart = uri.find(':', hostStart);
                if (portStart != std::string::npos) {
                    host = uri.substr(hostStart, portStart - hostStart);
                    port = std::stoi(uri.substr(portStart + 1));
                } else {
                    host = uri.substr(hostStart);
                }
            }
        }
    };

    static bool isSSL(const Uri& uri) {
        std::string schemeLower = uri.scheme;
        std::transform(schemeLower.begin(), schemeLower.end(), schemeLower.begin(), ::tolower);
        return schemeLower == "https" || schemeLower == "wss";
    }

    static int port(const Uri& uri) {
        if (uri.port == -1) {
            return isSSL(uri) ? 443 : 80;
        }
        return uri.port;
    }

        /**
         * @brief Joins the elements of a string vector into a single string, separated by a specified character.
         *
         * This function takes a vector of strings and a separator character and concatenates all the elements of the vector
         * into a single string. The separator character is used to separate each element in the resulting string.
         *
         * @param array The vector of strings to join.
         * @param separator The character used to separate each element in the resulting string.
         * @return A string containing all the elements of the input vector joined together with the specified separator.
         *
         * @note The function assumes that the input vector is not empty. If the input vector is empty, the function will return an empty string.
         *
         * @see LsUtils::split(const std::string&, char)
         * Implementation from http://commons.apache.org/proper/commons-lang/javadocs/api-3.8.1/org/apache/commons/lang3/StringUtils.html
         */
        static std::string join(const std::vector<std::string>& array, char separator) {
        std::ostringstream buf;
        for (size_t i = 0; i < array.size(); ++i) {
            if (i > 0) buf << separator;
            buf << array[i];
        }
        return buf.str();
    }

    static std::vector<std::string> split(const std::string& str, char separatorChar) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(str);
        while (std::getline(tokenStream, token, separatorChar)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    static bool Equals(const std::string& o1, const std::string& o2) {
        return o1 == o2;
    }

    static bool notEquals(const std::string& o1, const std::string& o2) {
        return !Equals(o1, o2);
    }
};

}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_LSUTILS_HPP
