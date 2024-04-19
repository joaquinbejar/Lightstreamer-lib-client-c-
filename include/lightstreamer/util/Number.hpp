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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_NUMBER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_NUMBER_HPP
#include <stdexcept>
#include <string>
#include <regex>

namespace lightstreamer::util {

    class Number {
    public:
        static constexpr bool ACCEPT_ZERO = true;
        static constexpr bool DONT_ACCEPT_ZERO = false;

        /**
         * Verifies that a given number is positive. Throws an exception if the check fails.
         * @param num The number to verify.
         * @param zeroAccepted Whether zero is considered a valid positive number.
         */
        static void verifyPositive(double num, bool zeroAccepted) {
            bool positive = isPositive(num, zeroAccepted);
            if (!positive) {
                if (zeroAccepted) {
                    throw std::invalid_argument("The given value is not valid. Use a positive number or 0");
                } else {
                    throw std::invalid_argument("The given value is not valid. Use a positive number");
                }
            }
        }

        /**
         * Checks if a number is positive based on the acceptance of zero.
         * @param num The number to check.
         * @param zeroAccepted Whether zero is considered a positive number.
         * @return True if the number is considered positive, otherwise false.
         */
        static bool isPositive(double num, bool zeroAccepted) {
            if (zeroAccepted) {
                return num >= 0;
            }
            return num > 0;
        }

        /**
         * Determines if a string represents a valid number.
         * @param num The string to check.
         * @return True if the string is a valid number, otherwise false.
         */
        static bool isNumber(const std::string& num) {
            static const std::regex pattern("^[+-]?\\d*\\.?\\d+$");
            return std::regex_match(num, pattern);
        }
    };

} // namespace lightstreamer::util


#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_NUMBER_HPP
