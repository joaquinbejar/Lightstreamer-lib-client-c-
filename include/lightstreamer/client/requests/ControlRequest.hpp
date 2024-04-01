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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_CONTROLREQUEST_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_CONTROLREQUEST_HPP
#include <memory>
#include <string>
#include <lightstreamer/client/requests/NumberedRequest.hpp>

namespace lightstreamer::client::requests {

    /**
     * Represents a control request with a predefined request name.
     */
    class ControlRequest : public NumberedRequest {
    public:
        /**
         * Sets the request name. This operation is skipped in ControlRequest as the name is fixed.
         *
         * @param name The name to set.
         */
        void setRequestName(const std::string& name) {
            // Skip. In ControlRequest, the request name is fixed.
        }

        /**
         * Gets the name of the request.
         *
         * @return Always returns "control".
         */
        std::string getRequestName() const override {
            return "control";
        }

        /**
         * Converts the request to its string representation, excluding transport-specific details.
         *
         * @return The transport unaware query string of the request.
         */
        std::string toString() const override {
            return this->TransportUnawareQueryString();
        }
    };
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_CONTROLREQUEST_HPP
