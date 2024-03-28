/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 28/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_ENCODINGUTILS_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_ENCODINGUTILS_HPP
#include <cassert>
#include <string>
#include <stdexcept>
#include <cstring>
#include <iomanip>
#include <sstream>

namespace lightstreamer::util {

    /**
     * Utility class for encoding-related operations.
     */
    class EncodingUtils {
    public:
        /**
         * Converts a string containing sequences as `%<hex digit><hex digit>` into a new string
         * where such sequences are transformed into UTF-8 encoded characters.
         * For example, the string "a%C3%A8" is converted to "aè" because the sequence 'C3 A8'
         * is the UTF-8 encoding of the character 'è'.
         * @param s The input string containing the encoded sequences.
         * @return A new string with the sequences decoded.
         */
        static std::string unquote(const std::string& s) {
            assert(!s.empty());
            try {
                std::string result;
                result.reserve(s.length()); // Optimize for space

                for (size_t i = 0; i < s.length(); ) {
                    if (s[i] == '%') {
                        if (i + 2 < s.length()) {
                            int firstHexDigit = hexToNum(s[i + 1]);
                            int secondHexDigit = hexToNum(s[i + 2]);
                            char decodedChar = static_cast<char>((firstHexDigit << 4) + secondHexDigit);
                            result.push_back(decodedChar);
                            i += 3;
                        }
                    } else {
                        result.push_back(s[i]);
                        ++i;
                    }
                }

                return result;
            } catch (const std::exception& e) {
                throw std::invalid_argument(e.what()); // Should not happen
            }
        }

    private:
        /**
         * Converts an ASCII-encoded hex digit to its numeric value.
         * @param ascii The ASCII character representing a hex digit.
         * @return The numeric value of the hex digit.
         */
        static int hexToNum(char ascii) {
            assert(std::isxdigit(ascii));

            if (ascii >= 'a') {
                return ascii - 'a' + 10;
            } else if (ascii >= 'A') {
                return ascii - 'A' + 10;
            } else {
                return ascii - '0';
            }
        }
    };

}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_ENCODINGUTILS_HPP
