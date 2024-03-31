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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_DESTROYREQUEST_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_DESTROYREQUEST_HPP
#include <memory>
#include <string>
#include <lightstreamer/client/requests/ControlRequest.hpp>

namespace lightstreamer::client::requests {

    /**
     * Represents a request to destroy a session with the server.
     *
     * @param targetServer The target server where the session will be destroyed.
     * @param sessionID The ID of the session to destroy.
     * @param closeReason The reason for closing the session.
     */
    class DestroyRequest : public ControlRequest {
    private:
        std::string session;

    public:
        /**
         * Constructs a DestroyRequest with specified parameters.
         *
         * @param targetServer The target server for the request.
         * @param sessionID The session ID associated with this request.
         * @param closeReason The reason for the session destruction.
         */
        DestroyRequest(const std::string& targetServer, const std::string& sessionID, const std::string& closeReason) {
            this->Server = targetServer;

            this->addParameter("LS_op", "destroy");

            this->session = sessionID;

            // The line below is commented out to match the C# version's behavior:
            // this->addParameter("LS_session", sessionID);

            if (!closeReason.empty()) {
                this->addParameter("LS_cause", closeReason);
            }
        }

        /**
         * Gets the session ID associated with this request.
         *
         * @return The session ID.
         */
        std::string getSession() const {
            return this->session;
        }
    };
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_DESTROYREQUEST_HPP
