/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 18/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_REQUESTSHELPER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_REQUESTSHELPER_HPP

#include <string>
#include <algorithm>

namespace lightstreamer::client::session {

    /**
     * Helper class for handling requests.
     */
    class RequestsHelper {
    private:
        static constexpr char HTTPS[] = "https://";
        static constexpr char HTTP[] = "http://";

    public:
        /**
         * Completes the control link URL by possibly adding a port and changing the protocol.
         *
         * @param extractFrom The original URL from which to extract the port.
         * @param controlLink The control link to be completed.
         * @return The completed control link URL.
         */
        static std::string completeControlLink(const std::string& extractFrom, std::string controlLink) {
            auto port = extractPort(extractFrom, extractFrom.find("://"));

            if (!port.empty()) {
                auto slIndex = controlLink.find('/');
                if (slIndex == std::string::npos) {
                    controlLink += port;
                } else {
                    controlLink = controlLink.substr(0, slIndex) + port + controlLink.substr(slIndex);
                }
            }

            if (extractFrom.find(HTTPS) == 0) {
                controlLink = HTTPS + controlLink;
            } else {
                controlLink = HTTP + controlLink;
            }

            if (controlLink.back() != '/') {
                controlLink += "/";
            }

            return controlLink;
        }

    private:
        /**
         * Extracts the port from the given URL.
         *
         * @param extractFrom The URL to extract the port from.
         * @param protLoc The location of the protocol in the URL.
         * @return The extracted port, or an empty string if no port is found.
         */
        static std::string extractPort(const std::string& extractFrom, size_t protLoc) {
            auto portStarts = extractFrom.find(':', protLoc + 1);
            if (portStarts == std::string::npos) {
                return "";
            }

            if (extractFrom.find(']') != std::string::npos) {
                portStarts = extractFrom.find("]:");
                if (portStarts == std::string::npos) {
                    return "";
                }
                portStarts += 1;
            } else if (portStarts != extractFrom.rfind(":")) {
                return "";
            }

            auto portEnds = extractFrom.find('/', protLoc + 3);
            if (portEnds != std::string::npos) {
                return extractFrom.substr(portStarts, portEnds - portStarts);
            } else {
                return extractFrom.substr(portStarts);
            }
        }
    };

} // namespace lightstreamer::client::session

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_REQUESTSHELPER_HPP
