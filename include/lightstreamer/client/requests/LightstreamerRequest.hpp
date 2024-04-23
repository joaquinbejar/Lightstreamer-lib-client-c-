/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 14/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_LIGHTSTREAMERREQUEST_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_LIGHTSTREAMERREQUEST_HPP
#include <sstream>
#include <iomanip>
#include <string>
#include <atomic>
#include <mutex>
#include <algorithm>

namespace lightstreamer::client::requests {

    class LightstreamerRequest {
    protected:
        std::ostringstream buffer;
        std::string targetServer;
        std::string session;
        static std::atomic<long> unique;

    public:
        LightstreamerRequest() : buffer(std::ostringstream()) {
            buffer.str("");
            buffer.clear();
        }

        virtual ~LightstreamerRequest() = default;

        void setServer(const std::string& value) {
            // value might be ignored (e.g., in case of a websocket connection)
            targetServer = value;
        }

        void setSession(const std::string& value) {
            session = value;
        }

        std::string getSession() const {
            return session;
        }

        virtual std::string getRequestName() const = 0;
        virtual void setRequestName(const std::string& value) = 0;

        static std::string encode(const std::string& value) {
            return percentEncodeTLCP(value);
        }

        /**
         * Helper function to add a parameter to the query string.
         * @param os The output string stream to which the parameter is appended.
         * @param key The parameter key.
         * @param value The parameter value.
         */
        static void addParameter(std::ostringstream& buffer, const std::string& name, const std::string& value) {
            buffer << name << "=" << encode(value) << "&";
        }

        static void addParameter(std::ostringstream& buffer, const std::string& name, double value) {
            buffer << name << "=" << std::to_string(value) << "&";
        }

        static void addParameter(std::ostringstream& buffer, const std::string& name, long value) {
            buffer << name << "=" << std::to_string(value) << "&";
        }


        void addUnique() {
            addParameter(this->buffer, "LS_unique", ++unique);
        }

        std::string getTransportAwareQueryString(const std::string& defaultSessionId, bool ackIsForced) {
            // This implementation is similar to getQueryStringBuilder logic in C#
            std::ostringstream result;
            result << buffer.str();
            if (!session.empty()) {
                bool sessionUnneeded = (!defaultSessionId.empty() && defaultSessionId == session);
                if (!sessionUnneeded) {
                    addParameter(result, "LS_session", session);
                }
            }
            if (result.tellp() == 0) {
                result << "\r\n";
            }
            return result.str();
        }

        std::string getTargetServer() const {
            return targetServer;
        }

        virtual bool isSessionRequest() const {
            return false;
        }

        /**
         * Gets the query string without considering the transport layer specifics.
         * @return A string representation of the query without transport-specific parameters.
         */
        std::string getTransportUnawareQueryString() {
            return getQueryStringBuilder("").str();
        }

    protected:
        static std::string percentEncodeTLCP(const std::string& str) {
            std::ostringstream encoded;
            for (char c : str) {
                if (isSpecial(c)) {
                    // Percent-encode special characters
                    encoded << '%' << std::uppercase << std::setw(2) << std::setfill('0')
                            << std::hex << (static_cast<int>(c) & 0xFF);
                } else {
                    // Directly append other characters
                    encoded << c;
                }
            }
            return encoded.str();
        }
        /**
         * Generates a query string for the request.
         * @param defaultSessionId The default session ID against which the current session is compared.
         * @return An output string stream filled with the constructed query string.
         */
        virtual std::ostringstream getQueryStringBuilder(const std::string& defaultSessionId) {
            std::ostringstream result;
            result << buffer.str();  // Append the current contents of the buffer

            if (!session.empty()) {
                bool sessionUnneeded = (!defaultSessionId.empty() && defaultSessionId == session);
                if (!sessionUnneeded) {
                    // Add the LS_session parameter only if necessary
                    addParameter(result, "LS_session", session);
                }
            }
            if (result.tellp() == 0) {
                // Ensure the query string is never empty as it is not allowed by the server
                result << "\r\n";
            }
            return result;
        }

    private:
        static bool isSpecial(char c) {
            return (c == '\r' || c == '\n' || c == '%' || c == '+' || c == '&' || c == '=');
        }

        static char hexDigits[16];

        static void initHexDigits() {
            for (int i = 0; i < 10; ++i) {
                hexDigits[i] = '0' + i;
            }
            for (int i = 10; i < 16; ++i) {
                hexDigits[i] = 'A' + (i - 10);
            }
        }
    };

    std::atomic<long> LightstreamerRequest::unique{0};
    char LightstreamerRequest::hexDigits[16];

} // namespace lightstreamer::client::requests


#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_LIGHTSTREAMERREQUEST_HPP
