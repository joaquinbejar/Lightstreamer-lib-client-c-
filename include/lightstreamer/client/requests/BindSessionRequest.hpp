/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 31/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_BINDSESSIONREQUEST_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_BINDSESSIONREQUEST_HPP

#include <string>
#include <map>
#include <lightstreamer/client/session/InternalConnectionOptions.hpp>

namespace lightstreamer::client::requests {

    class SessionRequest { // Assuming this is an existing base class
    protected:
        bool polling;
        long delay;
        std::map<std::string, std::string> parameters;

        // Example of a possible base class constructor
        SessionRequest(bool polling, long delay) : polling(polling), delay(delay) {}

    public:
        virtual std::string getRequestName() const = 0;

        virtual bool isSessionRequest() const = 0;

        // Assuming there's a mechanism to add parameters in the base class
        void addParameter(const std::string &key, const std::string &value) {
            parameters[key] = value;
        }
    };

    class BindSessionRequest : public SessionRequest {
    private:
        std::string server;
        std::string session;

    public:
        BindSessionRequest(const std::string &targetServer, const std::string &session, bool polling,
                           const std::string &cause,
                           const session::InternalConnectionOptions &options, long delay, bool addContentLength,
                           long maxReverseHeartbeatIntervalMs)
                : SessionRequest(polling, delay), server(targetServer), session(session) {

            if (polling) {
                addParameter("LS_polling", "true");
                addParameter("LS_polling_millis", std::to_string(options.pollingInterval + delay));
                addParameter("LS_idle_millis", std::to_string(options.idleTimeout));
            } else {
                if (options.keepaliveInterval > 0) {
                    addParameter("LS_keepalive_millis", std::to_string(options.keepaliveInterval));
                }

                if (maxReverseHeartbeatIntervalMs > 0) {
                    addParameter("LS_inactivity_millis", std::to_string(maxReverseHeartbeatIntervalMs));
                }

                if (addContentLength) {
                    addParameter("LS_content_length", std::to_string(options.contentLength));
                }
            }

            if (!cause.empty()) {
                addParameter("LS_cause", cause);
            }
        }

        std::string getRequestName() const override {
            return "bind_session";
        }

        bool isSessionRequest() const override {
            return true;
        }
    };

} // namespace requests


#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_BINDSESSIONREQUEST_HPP
