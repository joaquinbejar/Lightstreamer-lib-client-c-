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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_NUMBEREDREQUEST_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_NUMBEREDREQUEST_HPP
#include <memory>
#include <string>
#include <lightstreamer/client/requests/LightstreamerRequest.hpp>
#include <lightstreamer/util/IdGenerator.hpp>

namespace lightstreamer::client::requests {

    /**
     * Represents a numbered request with a unique request ID.
     */
    class NumberedRequest : public LightstreamerRequest {
    protected:
        long requestId;

    public:
        /**
         * Constructs a NumberedRequest and assigns a unique request ID.
         */
        NumberedRequest() : requestId(util::IdGenerator::NextRequestId()) {
            addParameter("LS_reqId", std::to_string(requestId));
        }

        /**
         * Gets the unique request ID of this NumberedRequest.
         *
         * @return The request ID.
         */
        long getRequestId() const {
            return requestId;
        }
    };
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_NUMBEREDREQUEST_HPP
