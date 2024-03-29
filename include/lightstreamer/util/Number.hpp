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

    constexpr bool ACCEPT_ZERO = true;
    constexpr bool DONT_ACCEPT_ZERO = false;

    bool isPositive(double num, bool zeroAccepted) {
        if (zeroAccepted) {
            if (num < 0) {
                return false;
            }
        } else if (num <= 0) {
            return false;
        }
        return true;
    }

    void verifyPositive(double num, bool zeroAccepted) {
        bool positive = isPositive(num, zeroAccepted);
        if (!positive) {
            if (zeroAccepted) {
                throw std::invalid_argument("The given value is not valid. Use a positive number or 0");
            } else {
                throw std::invalid_argument("The given value is not valid. Use a positive number");
            }
        }
    }

    // Static regex objects should be declared as const or constexpr in global scope or inside functions to avoid static initialization order issues
    bool isNumber(const std::string& num) {
        static const std::regex pattern(R"(^[+-]?\d*\.?\d+$)");
        return std::regex_match(num, pattern);
    }
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_NUMBER_HPP
