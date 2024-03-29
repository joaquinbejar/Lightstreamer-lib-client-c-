/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 29/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_STRINGHELPER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_STRINGHELPER_HPP

#include <string>
#include <vector>
#include <regex>

namespace StringHelper {

    std::string SubstringSpecial(const std::string &self, int start, int end) {
        return self.substr(start, end - start);
    }

    bool StartsWith(const std::string &self, const std::string &prefix, int toffset) {
        if (toffset + prefix.size() > self.size()) return false;
        return self.compare(toffset, prefix.size(), prefix) == 0;
    }

    std::vector<std::string>
    Split(const std::string &self, const std::string &regexDelimiter, bool trimTrailingEmptyStrings) {
        std::regex regex(regexDelimiter);
        std::sregex_token_iterator iter(self.begin(), self.end(), regex, -1);
        std::vector<std::string> splitArray(iter, std::sregex_token_iterator());

        if (trimTrailingEmptyStrings && !splitArray.empty()) {
            splitArray.erase(std::find_if(splitArray.rbegin(), splitArray.rend(), [](const std::string &s) {
                return !s.empty();
            }).base(), splitArray.end());
        }

        return splitArray;
    }

    std::string NewString(const std::vector<char> &bytes) {
        return std::string(bytes.begin(), bytes.end());
    }

    std::string NewString(const std::vector<char> &bytes, int index, int count) {
        if (index + count > static_cast<int>(bytes.size())) throw std::out_of_range("Index and count out of range");
        return std::string(bytes.begin() + index, bytes.begin() + index + count);
    }

    std::vector<char> GetBytes(const std::string &self) {
        return std::vector<char>(self.begin(), self.end());
    }


}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_STRINGHELPER_HPP
