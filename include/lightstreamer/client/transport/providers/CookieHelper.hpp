/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 4/4/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_COOKIEHELPER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_COOKIEHELPER_HPP
#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <lightstreamer/client/transport/providers/HttpCookie.hpp>
#include <lightstreamer/client/transport/providers/CookieContainer.hpp>

namespace lightstreamer::client::transport::providers {

    class CookieHelper {
    private:
        static inline std::mutex mutex;
        static std::vector<HttpCookie> custom_cookies;
        // Assuming CookieContainer is a class you have defined to mimic .NET's CookieContainer behavior
        static CookieContainer cookieHandler;

    public:
        static void logCookies(const std::string& messagePrefix, const std::vector<HttpCookie>& cookies) {
            std::string message = messagePrefix;
            for (const auto& cookie : cookies) {
                message += "\n    " + cookie.toString(); // Assuming HttpCookie has a toString() method
                // Assuming HttpCookie provides a Get method to access its properties
                message += " - domain " + cookie.Get("Domain");
                message += " - path " + cookie.Get("Path");
                message += " - version " + cookie.Get("Version");
            }
            // Placeholder for actual logging, replacing ILogger usage
            std::cout << message << std::endl;
        }

        static void addCookies(const std::string& uri, const std::vector<HttpCookie>& cookies) {
            if (cookies.empty()) {
                // Placeholder for warning log
                std::cout << "Received null reference for the cookies list to add." << std::endl;
                return;
            }
            std::lock_guard<std::mutex> lock(mutex);
            custom_cookies = cookies;

            // Placeholder for debug log before adding cookies
            for (const auto& cookie : cookies) {
                std::string tmpcookie;
                // Assuming HttpCookie supports iteration over its keys
                for (const auto& [key, value] : cookie) {
                    tmpcookie += key + "=" + value + "; ";
                }
                // Placeholder for setting cookies in a CookieContainer
                cookieHandler.SetCookies(uri, tmpcookie);
            }
            // Placeholder for debug log after adding cookies
        }

        static std::vector<HttpCookie> getCookies(const std::string& uri) {
            std::lock_guard<std::mutex> lock(mutex);
            if (!custom_cookies.empty()) {
                return custom_cookies;
            }
            return {}; // Returning an empty vector
        }

        static std::string getCookieHeader(const std::string& target) {
            // Placeholder for fetching cookie header from CookieContainer
            return ""; // Implement as per your CookieContainer's functionality
        }

        static void saveCookies(const std::string& uri, const std::string& cookieString) {
            if (cookieString.empty()) {
                // Placeholder for info log
                return;
            }
            // Placeholder for saving cookies
        }

        static bool CookieHandlerLocal() {
            std::lock_guard<std::mutex> lock(mutex);
            // Assuming cookieHandler is a pointer or can be checked for being set
            return true; // Modify based on actual implementation
        }

        static void reset() {
            std::lock_guard<std::mutex> lock(mutex);
            // Placeholder for discarding the custom CookieHandler and resetting state
        }
    };
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_COOKIEHELPER_HPP
