/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 12/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_CSOFFLINESTATUS_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_CSOFFLINESTATUS_HPP
#include <curl/curl.h>
#include <iostream>
#include <memory>
#include "Logger.hpp" // Assume this is your logging setup.
#include <lightstreamer/client/platform_data/offline/OfflineStatusInterface.hpp>

namespace lightstreamer::client::platform_data::offline  {

    class CSOfflineStatus : public OfflineStatusInterface {
    public:
        static Logger log; // Assume Logger is a class in your project for logging, similar to LogManager in the C# example.

        virtual bool isOffline(const std::string& server) {
            CURL *curl = curl_easy_init();
            if (!curl) {
                log.debug("CURL initialization failed");
                return true; // Assume offline if we can't even start CURL
            }

            curl_easy_setopt(curl, CURLOPT_URL, "http://clients3.google.com/generate_204");
            curl_easy_setopt(curl, CURLOPT_NOBODY, 1L); // We don't need the body for a status check
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); // Timeout for the operation, to not block indefinitely

            log.debug("IsOffline check now ...");

            CURLcode res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            if (res == CURLE_OK) {
                log.debug(" ... online, go!");
                return false;
            } else {
                log.debug(" ... offline!");
                return true;
            }
        }
    };

// Define the static logger
    Logger CSOfflineStatus::log = LogManager::getLogger("TRANSPORT_LOG");

} // namespace

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_CSOFFLINESTATUS_HPP
