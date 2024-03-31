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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_REVERSEHEARTBEATREQUEST_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_REVERSEHEARTBEATREQUEST_HPP
#include <memory>
#include <string>
#include <lightstreamer/client/requests/LightstreamerRequest.hpp>

namespace lightstreamer::client::requests {

    /**
     * Represents a reverse heartbeat request to keep the connection alive.
     */
    class ReverseHeartbeatRequest : public LightstreamerRequest {
    public:
        /**
         * Sets the request name. This operation is not applicable for ReverseHeartbeatRequest and is ignored.
         *
         * @param name The name to set.
         */
        void setRequestName(const std::string& name) override {
            // This is intentionally left empty as the request name for ReverseHeartbeatRequest is fixed.
        }

        /**
         * Gets the name of the request.
         *
         * @return Always returns "heartbeat".
         */
        std::string getRequestName() const override {
            return "heartbeat";
        }
    };
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_REVERSEHEARTBEATREQUEST_HPP
